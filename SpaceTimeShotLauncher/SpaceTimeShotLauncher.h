#pragma once
#include "global.h"
#include "ErrorHandler.h"
#include "ToggleFullscreen.h"
#include "CreateDevice.h"
#include "Render.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow);