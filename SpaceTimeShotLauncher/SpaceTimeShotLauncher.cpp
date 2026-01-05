#include "SpaceTimeShotLauncher.h"

// ------------------------------------------------------------
// Window Proc
// ------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SETCURSOR:
        if (g_IsFullscreen) {
            SetCursor(NULL); // Manually kill the cursor image
            return TRUE;     // Tell Windows we handled it
        }
        break; // Let DefWindowProc handle it for windowed mode
    case WM_SYSKEYDOWN:
        // Check if the key is Enter (VK_RETURN) and the ALT key is held
        if (wParam == VK_RETURN && (lParam & (1 << 29)))
        {
            ToggleFullscreen(hWnd);
            return 0;
        }
        return DefWindowProc(hWnd, msg, wParam, lParam);

    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_ESCAPE:
            PostQuitMessage(0);
            return 0;

        case VK_LEFT:  g_LastKey = L"LEFT"; break;
        case VK_RIGHT: g_LastKey = L"RIGHT"; break;
        case VK_UP:    g_LastKey = L"UP"; break;
        case VK_DOWN:  g_LastKey = L"DOWN"; break;

        default:
            // Allow other keys to fall through to DefWindowProc
            return DefWindowProc(hWnd, msg, wParam, lParam);
        }

        // This part only runs if it was one of your arrow keys
        if (!gameStarted)
        {
            gameStarted = true;
            g_StartTime = GetTickCount64();
            InitGame();
        }
        return 0;

    case WM_GETMINMAXINFO:
    {
        LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;

        // Optional: Scale limits by DPI for 2026 high-res monitors
        UINT dpi = GetDpiForWindow(hWnd);
        float scale = dpi / 96.0f;

        lpMMI->ptMinTrackSize.x = static_cast<LONG>(WINDOW_WIDTH * scale);
        lpMMI->ptMinTrackSize.y = static_cast<LONG>(WINDOW_HEIGHT * scale);

        return 0;
    }

    case WM_SIZE:
        // Ensure DirectX objects exist before attempting to resize
        if (g_SwapChain && g_D2DTarget)
        {
            // Release ALL device-dependent resources
            SAFE_RELEASE(g_BackgroundBitmap);
            SAFE_RELEASE(g_WhiteBrush);
            SAFE_RELEASE(g_GreenBrush);
            SAFE_RELEASE(g_D2DTarget);

            // Resize buffers to match new window size
            g_SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

            // Recreate resources for the new size
            CreateD2D();
            CreateText();
            LoadPNGFromResource(IDB_PNG1, &g_BackgroundBitmap);
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
    wcex.hCursor = nullptr;
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

    if (!g_hWnd) {
        ShowLastError(g_hWnd, L"CreateWindowW failed");

        return FALSE;
    }

    ShowWindow(g_hWnd, nCmdShow);
    ToggleFullscreen(g_hWnd);

    if (!CreateDevice() ||
        !CreateD2D() ||
        !CreateText() ||
        !LoadPNGFromResource(IDB_PNG1, &g_BackgroundBitmap)
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
    SAFE_RELEASE(g_SpaceShipBitmap);
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
