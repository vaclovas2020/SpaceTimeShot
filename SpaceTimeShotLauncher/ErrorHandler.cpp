#include "global.h"

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