#include "global.h"

void ToggleFullscreen(HWND hWnd)
{
    static WINDOWPLACEMENT wpPrev = { sizeof(wpPrev) };

    DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);

    if (dwStyle & WS_OVERLAPPEDWINDOW)
    {
        // Transition TO Fullscreen
        MONITORINFO mi = { sizeof(mi) };
        if (GetWindowPlacement(hWnd, &wpPrev) &&
            GetMonitorInfo(MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY), &mi))
        {
            SetWindowLong(hWnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW | WS_POPUP);
            SetWindowPos(hWnd, HWND_TOP,
                mi.rcMonitor.left, mi.rcMonitor.top,
                mi.rcMonitor.right - mi.rcMonitor.left,
                mi.rcMonitor.bottom - mi.rcMonitor.top,
                SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

            // Force the cursor to hide
            while (ShowCursor(FALSE) >= 0);

            g_IsFullscreen = true;
        }
    }
    else
    {
        // Transition TO Windowed
        SetWindowLong(hWnd, GWL_STYLE, dwStyle & ~WS_POPUP | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(hWnd, &wpPrev);
        SetWindowPos(hWnd, NULL, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

        // Force the cursor to show (when exiting fullscreen)
        while (ShowCursor(TRUE) < 0);

        g_IsFullscreen = false;
    }
}
