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

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
ULONG_PTR gdiplusToken;                         // GDI+ requires a token to start and stop

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
VOID                DestroyGlobalObjects();
VOID                ShowLastError(HWND hWnd, LPCWSTR functionName);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

// Global objects:
HBRUSH hBrushBackground;
Gdiplus::Bitmap* pGlobalBitmap;
HACCEL hAccelTable;
LPCWSTR g_lastKeyPressed = L"Press an arrow key to start";

// Time counter variables
ULONGLONG g_StartTime = 0;
WCHAR szCounterText[64] = L"00:00.00"; // Format: MM:SS.CC (Centiseconds)
BOOL gameStarted = false;

// FPS variables
LONGLONG g_Frequency;
LONGLONG g_LastTime;
float g_FrameTime; // Time in seconds for the last frame
WCHAR szFPS[20] = L"FPS: 0";


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDS_APP_CLASS, szWindowClass, MAX_LOADSTRING);

    // START GDI+
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // SHUTDOWN GDI+ (After message loop finishes)
    GdiplusShutdown(gdiplusToken);
    return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    hBrushBackground = CreateSolidBrush(RGB(0,0,0));


    WNDCLASSEXW wcex = {};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = hBrushBackground;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    RECT rect = { 0, 0, 1440, 900 };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOWCUSTOM, FALSE);

    HWND hWnd = CreateWindowW(
        szWindowClass, 
        szTitle, 
        WS_OVERLAPPEDWINDOWCUSTOM,
        0, 
        0, 
        rect.right - rect.left,         // Calculated width
        rect.bottom - rect.top,         // Calculated height
        nullptr, 
        nullptr, 
        hInstance, 
        nullptr
    );

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    SetTimer(hWnd, 1, 16, NULL);

    return TRUE;
}

VOID ShowLastError(HWND hWnd, LPCWSTR functionName) {
    // 1. Capture error immediately
    DWORD dwError = GetLastError();
    if (dwError == 0) return; // No error to show

    LPWSTR lpMsgBuf = nullptr;

    // 2. Format the system error message
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&lpMsgBuf, // Correct cast for ALLOCATE_BUFFER
        0, NULL
    );

    // 3. Show the message box
    // Use hWnd if available so the error box is modal to your window
    MessageBox(hWnd, lpMsgBuf, functionName, MB_OK | MB_ICONERROR);

    // 4. Cleanup allocated buffer
    LocalFree(lpMsgBuf);

    // 5. App-specific shutdown logic
    DestroyGlobalObjects();
    PostQuitMessage(1);
}

VOID DestroyGlobalObjects() {
    if (hBrushBackground != NULL) {
        DeleteObject(hBrushBackground);
        hBrushBackground = NULL;
    }

    if (pGlobalBitmap) {
        delete pGlobalBitmap;
        pGlobalBitmap = nullptr;
    }

    if (hAccelTable != NULL) {
        DestroyAcceleratorTable(hAccelTable);
        hAccelTable = NULL;
    }
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
        {
            HINSTANCE hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
            HRSRC hResInfo = FindResource(hInstance, MAKEINTRESOURCE(IDB_PNG1), L"PNG");

            QueryPerformanceFrequency((LARGE_INTEGER*)&g_Frequency);
            QueryPerformanceCounter((LARGE_INTEGER*)&g_LastTime);

            if (!hResInfo) {
                ShowLastError(hWnd, L"FindResource (Ensure IDB_PNG1 is type 'PNG' in .rc)");
                return -1; // WM_CREATE should return -1 on failure
            }

            DWORD dwSize = SizeofResource(hInstance, hResInfo);
            HGLOBAL hResData = LoadResource(hInstance, hResInfo);
            if (!hResData) {
                ShowLastError(hWnd, L"LoadResource");
                return -1;
            }

            void* pResBuffer = LockResource(hResData);
            HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, dwSize);
            if (!hMem) {
                ShowLastError(hWnd, L"GlobalAlloc");
                return -1;
            }

            void* pCopy = GlobalLock(hMem);
            if (pCopy == nullptr) {
                GlobalFree(hMem);
                ShowLastError(hWnd, L"GlobalLock");
                return -1;
            }

            memcpy(pCopy, pResBuffer, dwSize);
            GlobalUnlock(hMem);

            IStream* pStream = nullptr;
            if (CreateStreamOnHGlobal(hMem, TRUE, &pStream) != S_OK) {
                GlobalFree(hMem); // Free manually since stream wasn't created
                ShowLastError(hWnd, L"CreateStreamOnHGlobal");
                return -1;
            }

            // Load directly into the global pointer
            pGlobalBitmap = Gdiplus::Bitmap::FromStream(pStream);

            if (!pGlobalBitmap || pGlobalBitmap->GetLastStatus() != Gdiplus::Ok) {
                ShowLastError(hWnd, L"GDI+ PNG Load Failed");
                return -1;
            }

            pStream->Release(); // Memory is safe; GDI+ Bitmap keeps what it needs
        }
        break;

        case WM_ERASEBKGND:
            return TRUE;

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);

            if (wmEvent == 1) { // Accelerator

                switch (wmId) {
                    case ID_ACCELERATOR_VK_LEFT:  g_lastKeyPressed = L"LEFT"; gameStarted = true; break;
                    case ID_ACCELERATOR_VK_RIGHT: g_lastKeyPressed = L"RIGHT"; gameStarted = true; break;
                    case ID_ACCELERATOR_VK_UP:    g_lastKeyPressed = L"UP"; gameStarted = true;   break;
                    case ID_ACCELERATOR_VK_DOWN:  g_lastKeyPressed = L"DOWN"; gameStarted = true; break;
                }

                if (gameStarted && g_StartTime == 0) {
                    g_StartTime = GetTickCount64();
                }

                InvalidateRect(hWnd, NULL, FALSE);
                UpdateWindow(hWnd); // Force immediate redraw
                return 0;
            }
        }
        break;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            RECT rc;
            GetClientRect(hWnd, &rc);

            // --- FPS CALCULATION START ---
            LONGLONG currentTime;
            QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

            // Calculate elapsed time in seconds for the last frame
            g_FrameTime = (float)(currentTime - g_LastTime) / g_Frequency;

            // Update FPS counter every few frames (optional, prevents text flicker)
            static float timeAccumulator = 0.0f;
            timeAccumulator += g_FrameTime;

            if (timeAccumulator >= 0.1f) // Update FPS text ~10 times a second
            {
                float fps = 1.0f / g_FrameTime;
                swprintf_s(szFPS, L"FPS: %.1f", fps); // .1f for one decimal place
                timeAccumulator = 0.0f;
            }

            g_LastTime = currentTime;
            // --- FPS CALCULATION END ---

            // 1. Double Buffer Setup
            HDC backDC = CreateCompatibleDC(hdc);
            HBITMAP backBmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
            HBITMAP oldBackBmp = (HBITMAP)SelectObject(backDC, backBmp);

            // 2. Clear Background (prevents "trails" if window resizes)
            FillRect(backDC, &rc, hBrushBackground);

            Gdiplus::Graphics graphics(backDC);

            // 3. Draw with GDI+ for High Quality & Transparency
            // Use the GDI+ Graphics object to draw onto your backDC
            if (pGlobalBitmap) {
                graphics.DrawImage(pGlobalBitmap, 0, 0);
            }

            // --- INIT FONT OBJECTS
            Gdiplus::FontFamily fontFamily(L"Consolas");
            
            // --- DRAW LAST KEY PRESSED
            Gdiplus::Font font(&fontFamily, 24, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
            Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 255, 255, 255)); // White
            graphics.DrawString(g_lastKeyPressed, -1, &font, Gdiplus::PointF(20.0f, rc.bottom - 40.0f), &textBrush);
            
            // --- DRAW TIMER
            if (gameStarted) {
                Gdiplus::Font font2(&fontFamily, 36, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
                Gdiplus::SolidBrush textBrush2(Gdiplus::Color(255, 0, 255, 0)); // Green
                graphics.DrawString(szCounterText, -1, &font2, Gdiplus::PointF(20.0f, 20.0f), &textBrush2);

                Gdiplus::StringFormat format;
                // Align text to the top-right corner
                format.SetAlignment(Gdiplus::StringAlignmentFar);
                format.SetLineAlignment(Gdiplus::StringAlignmentNear);

                Gdiplus::RectF layoutRect(0.0f, 0.0f, (float)rc.right, (float)rc.bottom);
                graphics.DrawString(szFPS, -1, &font, layoutRect, &format, &textBrush);
            }

            // 4. Final Blit to Screen
            BitBlt(hdc, 0, 0, rc.right, rc.bottom, backDC, 0, 0, SRCCOPY);

            // 5. Cleanup
            SelectObject(backDC, oldBackBmp);
            DeleteObject(backBmp);
            DeleteDC(backDC);

            EndPaint(hWnd, &ps);
        }
        break;

        case WM_TIMER:
        {
            if (gameStarted) {
                ULONGLONG currentTime = GetTickCount64();
                ULONGLONG elapsed = currentTime - g_StartTime;

                // Calculate components
                UINT totalSeconds = (UINT)(elapsed / 1000);
                UINT minutes = totalSeconds / 60;
                UINT seconds = totalSeconds % 60;
                UINT centiseconds = (UINT)((elapsed % 1000) / 10); // Show 2 digits for milliseconds

                // Format: "01:23.45"
                swprintf_s(szCounterText, L"%02u:%02u.%02u", minutes, seconds, centiseconds);
            }

            // Refresh the screen
            InvalidateRect(hWnd, NULL, FALSE);
            UpdateWindow(hWnd); // Force immediate redraw
        }
        break;

        case WM_DESTROY:
        {
            DestroyGlobalObjects();
            PostQuitMessage(0);
        }
        break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}