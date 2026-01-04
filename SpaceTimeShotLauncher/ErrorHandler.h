#pragma once

void ShowError(const wchar_t* msg);
void ShowLastError(HWND hwnd, const wchar_t* context);
void ShowHRESULT(HWND hwnd, HRESULT hr, const wchar_t* context);