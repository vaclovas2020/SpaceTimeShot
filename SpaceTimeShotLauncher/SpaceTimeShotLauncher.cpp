// SpaceTimeShotLauncher.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "SpaceTimeShotLauncher.h"

using namespace Gdiplus;

#define MAX_LOADSTRING 100
#define WS_OVERLAPPEDWINDOWCUSTOM \
    (WS_OVERLAPPED     | \
     WS_CAPTION        | \
     WS_SYSMENU        | \
     WS_MINIMIZEBOX)

// ------------------------------------------------------------
// Globals
// ------------------------------------------------------------
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
ULONG_PTR gdiplusToken = 0;

HACCEL hAccelTable = nullptr;
HBRUSH hBrushBackground = nullptr;

// Back buffer
HDC     g_BackDC = nullptr;
HBITMAP g_BackBitmap = nullptr;

// GDI+
std::unique_ptr<Bitmap> g_BackgroundImage;
std::unique_ptr<Font>   g_FontSmall;
std::unique_ptr<Font>   g_FontLarge;
std::unique_ptr<SolidBrush> g_WhiteBrush;
std::unique_ptr<SolidBrush> g_GreenBrush;

// State
LPCWSTR g_lastKeyPressed = L"Press an arrow key to start";
bool gameStarted = false;
ULONGLONG g_StartTime = 0;
WCHAR szCounterText[64] = L"00:00.00";
WCHAR szFPS[32] = L"FPS: 0";

// Timing
LONGLONG g_Frequency = 0;
LONGLONG g_LastTime = 0;
float g_FrameTime = 0.0f;

// FPS averaging
float g_FpsAccum = 0.0f;
int   g_FpsFrames = 0;

// ------------------------------------------------------------
// Forward declarations
// ------------------------------------------------------------
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

void CreateBackBuffer(HWND hWnd);
void DestroyBackBuffer();
void UpdateGame();
void Render(HWND hWnd);
void DestroyGlobalObjects();
void ShowLastError(HWND hWnd, LPCWSTR fn);

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

    GdiplusStartupInput gsi;
    GdiplusStartup(&gdiplusToken, &gsi, nullptr);

    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
        return FALSE;

    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));

    QueryPerformanceFrequency((LARGE_INTEGER*)&g_Frequency);
    QueryPerformanceCounter((LARGE_INTEGER*)&g_LastTime);

    MSG msg{};
    HWND hWnd = FindWindow(szWindowClass, nullptr);

    while (true)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                goto shutdown;

            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        UpdateGame();
        InvalidateRect(hWnd, nullptr, FALSE);
        Sleep(1);
    }

shutdown:
    DestroyGlobalObjects();
    GdiplusShutdown(gdiplusToken);
    return (int)msg.wParam;
}

// ------------------------------------------------------------
// Error Handling
// ------------------------------------------------------------
void ShowLastError(HWND hWnd, LPCWSTR fn)
{
    DWORD err = GetLastError();
    if (!err) return;

    LPWSTR msg = nullptr;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        nullptr, err, 0, (LPWSTR)&msg, 0, nullptr);

    MessageBox(hWnd, msg, fn, MB_ICONERROR | MB_OK);
    LocalFree(msg);
}

// ------------------------------------------------------------
// Cleanup
// ------------------------------------------------------------
void DestroyGlobalObjects()
{
    DestroyBackBuffer();

    g_BackgroundImage.reset();
    g_FontSmall.reset();
    g_FontLarge.reset();
    g_WhiteBrush.reset();
    g_GreenBrush.reset();

    if (hBrushBackground)
    {
        DeleteObject(hBrushBackground);
        hBrushBackground = nullptr;
    }

    if (hAccelTable)
    {
        DestroyAcceleratorTable(hAccelTable);
        hAccelTable = nullptr;
    }
}

// ------------------------------------------------------------
// Window Class
// ------------------------------------------------------------
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    hBrushBackground = CreateSolidBrush(RGB(0, 0, 0));

    WNDCLASSEXW wcex{};
    wcex.cbSize = sizeof(wcex);
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wcex.hIconSm = wcex.hIcon;
    wcex.hbrBackground = nullptr;
    wcex.lpszClassName = szWindowClass;

    return RegisterClassExW(&wcex);
}

// ------------------------------------------------------------
// Init Instance
// ------------------------------------------------------------
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    RECT rc = { 0,0,1440,900 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOWCUSTOM, FALSE);

    HWND hWnd = CreateWindowW(
        szWindowClass, szTitle,
        WS_OVERLAPPEDWINDOWCUSTOM,
        CW_USEDEFAULT, 0,
        rc.right - rc.left,
        rc.bottom - rc.top,
        nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        ShowLastError(nullptr, L"CreateWindowW");
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}

// ------------------------------------------------------------
// Back Buffer
// ------------------------------------------------------------
void CreateBackBuffer(HWND hWnd)
{
    DestroyBackBuffer();

    RECT rc;
    GetClientRect(hWnd, &rc);

    HDC hdc = GetDC(hWnd);
    g_BackDC = CreateCompatibleDC(hdc);
    g_BackBitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);

    if (!g_BackDC || !g_BackBitmap)
    {
        ShowLastError(hWnd, L"CreateBackBuffer");
        ReleaseDC(hWnd, hdc);
        return;
    }

    SelectObject(g_BackDC, g_BackBitmap);
    ReleaseDC(hWnd, hdc);
}

void DestroyBackBuffer()
{
    if (g_BackBitmap) { DeleteObject(g_BackBitmap); g_BackBitmap = nullptr; }
    if (g_BackDC) { DeleteDC(g_BackDC); g_BackDC = nullptr; }
}

// ------------------------------------------------------------
// Update
// ------------------------------------------------------------
void UpdateGame()
{
    LONGLONG now;
    QueryPerformanceCounter((LARGE_INTEGER*)&now);
    g_FrameTime = float(now - g_LastTime) / g_Frequency;
    g_LastTime = now;

    g_FpsAccum += g_FrameTime;
    g_FpsFrames++;

    if (g_FpsAccum >= 0.5f)
    {
        swprintf_s(szFPS, L"FPS: %.1f", g_FpsFrames / g_FpsAccum);
        g_FpsAccum = 0;
        g_FpsFrames = 0;
    }

    if (gameStarted)
    {
        ULONGLONG e = GetTickCount64() - g_StartTime;
        swprintf_s(szCounterText, L"%02llu:%02llu.%02llu",
            e / 60000, (e / 1000) % 60, (e % 1000) / 10);
    }
}

// ------------------------------------------------------------
// Render
// ------------------------------------------------------------
void Render(HWND hWnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);

    RECT rc;
    GetClientRect(hWnd, &rc);

    FillRect(g_BackDC, &rc, hBrushBackground);
    Graphics g(g_BackDC);

    if (g_BackgroundImage)
        g.DrawImage(g_BackgroundImage.get(), 0, 0);

    g.DrawString(g_lastKeyPressed, -1, g_FontSmall.get(),
        PointF(20, rc.bottom - 40), g_WhiteBrush.get());

    if (gameStarted)
    {
        g.DrawString(szCounterText, -1, g_FontLarge.get(),
            PointF(20, 20), g_GreenBrush.get());

        StringFormat fmt;
        fmt.SetAlignment(StringAlignmentFar);
        RectF r(0, 0, (REAL)rc.right, (REAL)rc.bottom);
        g.DrawString(szFPS, -1, g_FontSmall.get(), r, &fmt, g_WhiteBrush.get());
    }

    BitBlt(hdc, 0, 0, rc.right, rc.bottom, g_BackDC, 0, 0, SRCCOPY);
    EndPaint(hWnd, &ps);
}

// ------------------------------------------------------------
// WndProc
// ------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        CreateBackBuffer(hWnd);

        HRSRC res = FindResource(hInst, MAKEINTRESOURCE(IDB_PNG1), L"PNG");
        if (!res) { ShowLastError(hWnd, L"FindResource"); return -1; }

        DWORD size = SizeofResource(hInst, res);
        HGLOBAL data = LoadResource(hInst, res);
        void* ptr = LockResource(data);

        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
        memcpy(GlobalLock(hMem), ptr, size);
        GlobalUnlock(hMem);

        IStream* stream = nullptr;
        CreateStreamOnHGlobal(hMem, TRUE, &stream);
        g_BackgroundImage.reset(Bitmap::FromStream(stream));
        stream->Release();

        FontFamily ff(L"Consolas");
        g_FontSmall = std::make_unique<Font>(&ff, 24, FontStyleBold, UnitPixel);
        g_FontLarge = std::make_unique<Font>(&ff, 36, FontStyleBold, UnitPixel);
        g_WhiteBrush = std::make_unique<SolidBrush>(Color(255, 255, 255));
        g_GreenBrush = std::make_unique<SolidBrush>(Color(255, 0, 255, 0));
    }
    return 0;

    case WM_SIZE:
        CreateBackBuffer(hWnd);
        return 0;

    case WM_ERASEBKGND:
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_ACCELERATOR_VK_LEFT:  g_lastKeyPressed = L"LEFT"; break;
        case ID_ACCELERATOR_VK_RIGHT: g_lastKeyPressed = L"RIGHT"; break;
        case ID_ACCELERATOR_VK_UP:    g_lastKeyPressed = L"UP"; break;
        case ID_ACCELERATOR_VK_DOWN:  g_lastKeyPressed = L"DOWN"; break;
        default: return DefWindowProc(hWnd, msg, wParam, lParam);
        }

        if (!gameStarted)
        {
            gameStarted = true;
            g_StartTime = GetTickCount64();
        }
        return 0;

    case WM_PAINT:
        Render(hWnd);
        return 0;

    case WM_DESTROY:
        DestroyGlobalObjects();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}
