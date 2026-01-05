#include "global.h"

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

// Images
ID2D1Bitmap* g_BackgroundBitmap = nullptr; // background
ID2D1Bitmap* g_SpaceShipBitmap = nullptr; // spaceship game object (main character)

// Game state
bool        gameStarted = false;
bool        g_IsFullscreen = false;
ULONGLONG   g_StartTime = 0;
std::wstring g_LastKey = L"Press an arrow key to start";

// FPS
LARGE_INTEGER g_Freq;
LARGE_INTEGER g_LastTime;
float g_FpsAccum = 0.0f;
int   g_FpsFrames = 0;
float g_CurrentFPS = 0.0f;

// Game Objects
Player* g_Player = nullptr;