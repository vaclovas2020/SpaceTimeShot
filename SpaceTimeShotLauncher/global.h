#pragma once

#include "framework.h"
#include "resource.h"

#define SAFE_RELEASE(x) if (x) { x->Release(); x = nullptr; }

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720

#define MAX_LOADSTRING 100

// ------------------------------------------------------------
// Globals
// ------------------------------------------------------------
extern HWND g_hWnd;
extern HINSTANCE g_hInst;
extern WCHAR szTitle[MAX_LOADSTRING];
extern WCHAR szWindowClass[MAX_LOADSTRING];

// DXGI / D3D
extern ID3D11Device* g_Device;
extern ID3D11DeviceContext* g_Context;
extern IDXGISwapChain* g_SwapChain;

// Direct2D
extern ID2D1Factory* g_D2DFactory;
extern ID2D1RenderTarget* g_D2DTarget;

// DirectWrite
extern IDWriteFactory* g_WriteFactory;
extern IDWriteTextFormat* g_FontSmall;
extern IDWriteTextFormat* g_FontLarge;

// Brushes
extern ID2D1SolidColorBrush* g_WhiteBrush;
extern ID2D1SolidColorBrush* g_GreenBrush;

// Image
extern ID2D1Bitmap* g_BackgroundBitmap;

// Game state
extern bool        gameStarted;
extern bool        g_IsFullscreen;
extern ULONGLONG   g_StartTime;
extern std::wstring g_LastKey;

// FPS
extern LARGE_INTEGER g_Freq;
extern LARGE_INTEGER g_LastTime;
extern float g_FpsAccum;
extern int   g_FpsFrames;
extern float g_CurrentFPS;
