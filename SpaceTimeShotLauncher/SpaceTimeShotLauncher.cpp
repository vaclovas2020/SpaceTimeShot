// SpaceTimeShotLauncher.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "SpaceTimeShotLauncher.h"

#define SAFE_RELEASE(x) if (x) { x->Release(); x = nullptr; }

#define WINDOW_WIDTH  1440
#define WINDOW_HEIGHT 900

#define MAX_LOADSTRING 100

// ------------------------------------------------------------
// Globals
// ------------------------------------------------------------
HWND g_hWnd = nullptr;
HINSTANCE g_hInst = nullptr;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

// DXGI / D3D
ID3D11Device* g_Device = nullptr;
ID3D11DeviceContext* g_Context = nullptr;
IDXGISwapChain* g_SwapChain = nullptr;

// Direct2D
ID2D1Factory* g_D2DFactory = nullptr;
ID2D1RenderTarget* g_D2DTarget = nullptr;

// DirectWrite
IDWriteFactory* g_WriteFactory = nullptr;
IDWriteTextFormat* g_FontSmall = nullptr;
IDWriteTextFormat* g_FontLarge = nullptr;

// Brushes
ID2D1SolidColorBrush* g_WhiteBrush = nullptr;
ID2D1SolidColorBrush* g_GreenBrush = nullptr;

// Image
ID2D1Bitmap* g_BackgroundBitmap = nullptr;

// Game state
bool        gameStarted = false;
ULONGLONG   g_StartTime = 0;
std::wstring g_LastKey = L"Press an arrow key to start";

// FPS
LARGE_INTEGER g_Freq;
LARGE_INTEGER g_LastTime;
float g_FpsAccum = 0.0f;
int   g_FpsFrames = 0;
float g_CurrentFPS = 0.0f;

// ------------------------------------------------------------
// Error helper
// ------------------------------------------------------------
void ShowError(const wchar_t* msg)
{
    MessageBoxW(g_hWnd, msg, L"Error", MB_ICONERROR);
}

void ShowLastError(HWND hwnd, const wchar_t* context)
{
    DWORD err = GetLastError();
    if (err == 0)
    {
        MessageBoxW(hwnd, context, L"Win32 Error", MB_ICONERROR);
        return;
    }

    wchar_t* msg = nullptr;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&msg,
        0,
        nullptr);

    wchar_t full[1024];
    swprintf_s(full, L"%s\n\nError %lu:\n%s", context, err, msg);

    MessageBoxW(hwnd, full, L"Win32 Error", MB_ICONERROR);
    LocalFree(msg);
}


void ShowHRESULT(HWND hwnd, HRESULT hr, const wchar_t* context)
{
    wchar_t* msg = nullptr;

    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        hr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&msg,
        0,
        nullptr);

    wchar_t full[1024];
    swprintf_s(
        full,
        L"%s\n\nHRESULT: 0x%08X\n%s",
        context,
        hr,
        msg ? msg : L"(No system message)");

    MessageBoxW(hwnd, full, L"DirectX / COM Error", MB_ICONERROR);

    if (msg) LocalFree(msg);
}


// ------------------------------------------------------------
// Create Device & SwapChain (REAL VSYNC)
// ------------------------------------------------------------
bool CreateDevice()
{
    // Define initial flags (e.g., for Direct2D compatibility)
    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    #if defined(_DEBUG)
        // Add the debug flag only for debug builds in 2026
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

    DXGI_SWAP_CHAIN_DESC scd{};
    scd.BufferCount = 2;                                    // Flip models require 2+
    scd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = g_hWnd;                              // Ensure this is not NULL
    scd.SampleDesc.Count = 1;                               // Flip models DO NOT support MSAA
    scd.SampleDesc.Quality = 0;                             // Must be 0
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scd.Flags = 0;                                          // Avoid experimental flags during init

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        featureLevels, ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &scd,
        &g_SwapChain,
        &g_Device,
        nullptr,
        &g_Context);

    if (FAILED(hr))
        ShowHRESULT(g_hWnd, hr, L"D3D11CreateDeviceAndSwapChain() failed");

    return SUCCEEDED(hr);
}

// ------------------------------------------------------------
// Create Direct2D Target
// ------------------------------------------------------------
bool CreateD2D()
{
    HRESULT hr = S_OK;

    if (g_D2DFactory == nullptr) {
        hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_D2DFactory);
        if (FAILED(hr)) return false;
    }

    if (FAILED(hr)) {
        ShowHRESULT(g_hWnd, hr, L"D2D1CreateFactory() failed");
        return false;
    }

    IDXGISurface* surface = nullptr;
    g_SwapChain->GetBuffer(0, IID_PPV_ARGS(&surface));

    D2D1_RENDER_TARGET_PROPERTIES props =
        D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_HARDWARE,
            D2D1::PixelFormat(
                DXGI_FORMAT_B8G8R8A8_UNORM,
                D2D1_ALPHA_MODE_PREMULTIPLIED));

    if (surface == nullptr) {
        ShowError(L"surface is nullptrt");

        return false;
    }

    hr = g_D2DFactory->CreateDxgiSurfaceRenderTarget(
        surface,
        &props,
        &g_D2DTarget);

    surface->Release();

    if (FAILED(hr)) {
        ShowHRESULT(g_hWnd, hr, L"CreateDxgiSurfaceRenderTarget() failed");
    }

    return SUCCEEDED(hr);
}

// ------------------------------------------------------------
// Load PNG from Resource
// ------------------------------------------------------------
bool LoadPNGFromResource(int resourceId)
{
    IWICImagingFactory* wic = nullptr;
    IWICStream* stream = nullptr;
    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICFormatConverter* converter = nullptr;

    // Locate resource
    HRSRC hrsrc = FindResource(
        g_hInst,
        MAKEINTRESOURCE(resourceId),
        L"PNG");

    if (!hrsrc) return false;

    HGLOBAL hglob = LoadResource(g_hInst, hrsrc);
    if (!hglob) return false;

    void* data = LockResource(hglob);
    DWORD size = SizeofResource(g_hInst, hrsrc);
    if (!data || !size) return false;

    // Create WIC factory
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&wic));
    
    if (FAILED(hr))
    {
        ShowHRESULT(g_hWnd, hr, L"CoCreateInstance() failed");

        goto cleanup;
    }

    // Create stream from memory
    hr = wic->CreateStream(&stream);
    
    if (FAILED(hr))
    {
        ShowHRESULT(g_hWnd, hr, L"CreateStream() failed");

        goto cleanup;
    }

    hr = stream->InitializeFromMemory(
        reinterpret_cast<BYTE*>(data),
        size);
    
    if (FAILED(hr))
    {
        ShowHRESULT(g_hWnd, hr, L"InitializeFromMemory() failed");

        goto cleanup;
    }

    // Decode PNG
    hr = wic->CreateDecoderFromStream(
        stream,
        nullptr,
        WICDecodeMetadataCacheOnLoad,
        &decoder);
    
    if (FAILED(hr))
    {
        ShowHRESULT(g_hWnd, hr, L"CreateDecoderFromStream() failed");

        goto cleanup;
    }

    hr = decoder->GetFrame(0, &frame);
    
    if (FAILED(hr))
    {
        ShowHRESULT(g_hWnd, hr, L"decoder->GetFrame() failed");

        goto cleanup;
    }

    // Convert to 32bpp premultiplied BGRA
    hr = wic->CreateFormatConverter(&converter);
   
    if (FAILED(hr))
    {
        ShowHRESULT(g_hWnd, hr, L"wic->CreateFormatConverter() failed");

        goto cleanup;
    }

    hr = converter->Initialize(
        frame,
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeCustom);
   
    if (FAILED(hr))
    {
        ShowHRESULT(g_hWnd, hr, L"converter->Initialize() failed");

        goto cleanup;
    }

    // Create Direct2D bitmap
    hr = g_D2DTarget->CreateBitmapFromWicBitmap(
        converter,
        nullptr,
        &g_BackgroundBitmap);

    if (FAILED(hr))
    {
        ShowHRESULT(g_hWnd, hr, L"g_D2DTarget->CreateBitmapFromWicBitmap() failed");

        goto cleanup;
    }

cleanup:
    SAFE_RELEASE(converter);
    SAFE_RELEASE(frame);
    SAFE_RELEASE(decoder);
    SAFE_RELEASE(stream);
    SAFE_RELEASE(wic);

    return SUCCEEDED(hr) && g_BackgroundBitmap != nullptr;
}

// ------------------------------------------------------------
// Init DirectWrite + Brushes
// ------------------------------------------------------------
bool CreateText()
{
    HRESULT hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        (IUnknown**)&g_WriteFactory);

    if (FAILED(hr))
    {
        ShowHRESULT(g_hWnd, hr, L"DWriteCreateFactory() failed");

        return false;
    }

    if (FAILED(
        g_WriteFactory->CreateTextFormat(
            L"Consolas", nullptr,
            DWRITE_FONT_WEIGHT_BOLD,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            24, L"",
            &g_FontSmall)
        )
    ) {
        ShowHRESULT(g_hWnd, hr, L"CreateTextFormat() failed");

        return false;
    }

    if (FAILED(
        g_WriteFactory->CreateTextFormat(
            L"Consolas", nullptr,
            DWRITE_FONT_WEIGHT_BOLD,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            36, L"",
            &g_FontLarge)
        )
    ) {
        ShowHRESULT(g_hWnd, hr, L"CreateTextFormat() failed");

        return false;
    }

    if (FAILED(
        g_D2DTarget->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::White),
            &g_WhiteBrush)
        )
    ) {
        ShowHRESULT(g_hWnd, hr, L"CreateTextFormat() failed");

        return false;
    }

    if (FAILED(
        g_D2DTarget->CreateSolidColorBrush(
            D2D1::ColorF(0, 1, 0),
            &g_GreenBrush)
        )
    ) {
        ShowHRESULT(g_hWnd, hr, L"CreateTextFormat() failed");

        return false;
    }

    return true;
}

// ------------------------------------------------------------
// Update + FPS
// ------------------------------------------------------------
void Update()
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);

    float dt = float(now.QuadPart - g_LastTime.QuadPart) / g_Freq.QuadPart;
    g_LastTime = now;

    g_FpsAccum += dt;
    g_FpsFrames++;

    if (g_FpsAccum >= 0.5f)
    {
        g_CurrentFPS = g_FpsFrames / g_FpsAccum;
        g_FpsAccum = 0;
        g_FpsFrames = 0;
    }
}

// ------------------------------------------------------------
// Render (REAL VSYNC)
// ------------------------------------------------------------
void Render()
{
    if (!g_D2DTarget) return;

    g_D2DTarget->BeginDraw();
    g_D2DTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

    if (g_BackgroundBitmap)
    {
        D2D1_SIZE_F s = g_D2DTarget->GetSize();
        g_D2DTarget->DrawBitmap(
            g_BackgroundBitmap,
            D2D1::RectF(0, 0, s.width, s.height));
    }

    // Bottom-left key text
    D2D1_SIZE_F size = g_D2DTarget->GetSize();
    D2D1_RECT_F bottom =
        D2D1::RectF(20, size.height - 50, size.width, size.height);

    g_D2DTarget->DrawTextW(
        g_LastKey.c_str(),
        (UINT32)g_LastKey.size(),
        g_FontSmall,
        bottom,
        g_WhiteBrush);

    if (gameStarted)
    {
        ULONGLONG e = GetTickCount64() - g_StartTime;
        wchar_t timer[64];
        swprintf_s(timer, L"%02llu:%02llu.%02llu",
            e / 60000, (e / 1000) % 60, (e % 1000) / 10);

        g_D2DTarget->DrawTextW(
            timer, wcslen(timer),
            g_FontLarge,
            D2D1::RectF(20, 20, 400, 100),
            g_GreenBrush);

        wchar_t fps[32];
        swprintf_s(fps, L"FPS: %.1f", g_CurrentFPS);

        g_D2DTarget->DrawTextW(
            fps, wcslen(fps),
            g_FontSmall,
            D2D1::RectF(size.width - 200, 20, size.width, 80),
            g_WhiteBrush);
    }

    g_D2DTarget->EndDraw();

    // 🔥 REAL VSYNC
    g_SwapChain->Present(1, 0);
}

// ------------------------------------------------------------
// Window Proc
// ------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_LEFT:  g_LastKey = L"LEFT"; break;
        case VK_RIGHT: g_LastKey = L"RIGHT"; break;
        case VK_UP:    g_LastKey = L"UP"; break;
        case VK_DOWN:  g_LastKey = L"DOWN"; break;
        default: return 0;
        }

        if (!gameStarted)
        {
            gameStarted = true;
            g_StartTime = GetTickCount64();
        }
        return 0;

    case WM_GETMINMAXINFO:
    {
        // lParam is a pointer to a MINMAXINFO structure
        LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;

        // Set the minimum width and height in pixels
        // ptMinTrackSize is the smallest the window can be dragged
        lpMMI->ptMinTrackSize.x = 1280; // Minimum width
        lpMMI->ptMinTrackSize.y = 720; // Minimum height

        return 0; // Return 0 to indicate we processed this message
    }

    case WM_SIZE:
        if (g_SwapChain && g_D2DTarget) // Ensure swap chain exists first
        {
            // 1. Release ALL D2D resources that depend on the swap chain's buffers
            SAFE_RELEASE(g_BackgroundBitmap); // Bitmaps are bound to the target
            SAFE_RELEASE(g_WhiteBrush);       // Brushes are bound to the target
            SAFE_RELEASE(g_GreenBrush);
            SAFE_RELEASE(g_D2DTarget);        // Release the target itself

            // 2. Clear the D3D context state to ensure no internal references remain
            if (g_Context) g_Context->ClearState();

            // 3. Resize the buffers
            // Use 0, 0 to tell DXGI to use the actual window client area size
            HRESULT hr = g_SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

            if (FAILED(hr)) {
                // If this fails, the debug layer will trigger your __debugbreak()
                ShowHRESULT(hWnd, hr, L"ResizeBuffers Failed");
            }

            // 4. Recreate the target and resources
            CreateD2D();
            CreateText();
            LoadPNGFromResource(IDB_PNG1);
        }
        return 0;


    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// ------------------------------------------------------------
// Entry Point
// ------------------------------------------------------------
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDS_APP_CLASS, szWindowClass, MAX_LOADSTRING);

    g_hInst = hInstance;

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    if (FAILED(hr)) {
        ShowHRESULT(g_hWnd, hr, L"CoInitializeEx failed");
        return 0;
    }

    QueryPerformanceFrequency(&g_Freq);
    QueryPerformanceCounter(&g_LastTime);

    WNDCLASSEXW wcex{};
    wcex.cbSize = sizeof(wcex);
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wcex.hIconSm = wcex.hIcon;
    wcex.hbrBackground = nullptr;
    wcex.lpszClassName = szWindowClass;

    RegisterClassExW(&wcex);

    g_hWnd = CreateWindowW(
        szWindowClass, 
        szTitle,
        WS_OVERLAPPEDWINDOW,
        0, 0,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        nullptr, nullptr, hInstance, nullptr);

    if (! g_hWnd) {
        ShowLastError(g_hWnd, L"CreateWindowW failed");

        return FALSE;
    }

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    if (!CreateDevice() ||
        !CreateD2D() ||
        !CreateText() ||
        !LoadPNGFromResource(IDB_PNG1)
        ) {
        return 0;
    }

    MSG msg{};
    while (true)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                goto shutdown;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        Update();
        Render();
    }

shutdown:
    SAFE_RELEASE(g_BackgroundBitmap);
    SAFE_RELEASE(g_WhiteBrush);
    SAFE_RELEASE(g_GreenBrush);
    SAFE_RELEASE(g_FontSmall);
    SAFE_RELEASE(g_FontLarge);
    SAFE_RELEASE(g_WriteFactory);
    SAFE_RELEASE(g_D2DTarget);
    SAFE_RELEASE(g_D2DFactory);
    SAFE_RELEASE(g_SwapChain);
    SAFE_RELEASE(g_Context);
    SAFE_RELEASE(g_Device);

    CoUninitialize();
    return 0;
}