#include "global.h"
#include "ErrorHandler.h"
#include "Render.h"

std::vector<Bullet*> g_Bullets;

void HandleShooting() {
    // Only spawn if Space is pressed (0x8000 check)
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
        // Position bullet at the center-top of the player ship
        float bulletX = g_Player->GetX() + (256.0f / 2.0f);
        float bulletY = g_Player->GetY();

        g_Bullets.push_back(new Bullet(bulletX, bulletY));
    }
}

void InitGame() {
    // 1. Get the real window dimensions
    RECT rc;
    GetClientRect(g_hWnd, &rc);
    float screenWidth = (float)(rc.right - rc.left);
    float screenHeight = (float)(rc.bottom - rc.top);

    // 2. Define player dimensions
    float playerWidth = 256.0f;
    float playerHeight = 256.0f;

    // 3. Calculate bottom-center position
    float startX = (screenWidth - playerWidth) / 2.0f;
    float startY = screenHeight - playerHeight;

    // 4. Initialize player
    g_Player = new Player(startX, startY);

    ID2D1Bitmap* playerBmp = nullptr;
    if (LoadPNGFromResource(IDB_SPACESHIP, &g_SpaceShipBitmap)) {
        g_Player->SetBitmap(g_SpaceShipBitmap);
    }
}

// ------------------------------------------------------------
// Load PNG from Resource
// ------------------------------------------------------------
// Added ID2D1Bitmap** outBitmap as a parameter
bool LoadPNGFromResource(int resourceId, ID2D1Bitmap** ppBitmap)
{
    // Important: Initialize the output pointer to nullptr 
    if (ppBitmap == nullptr) return false;
    *ppBitmap = nullptr;

    IWICImagingFactory* wic = nullptr;
    IWICStream* stream = nullptr;
    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICFormatConverter* converter = nullptr;

    HRSRC hrsrc = FindResource(g_hInst, MAKEINTRESOURCE(resourceId), L"PNG");
    if (!hrsrc) return false;

    HGLOBAL hglob = LoadResource(g_hInst, hrsrc);
    if (!hglob) return false;

    void* data = LockResource(hglob);
    DWORD size = SizeofResource(g_hInst, hrsrc);
    if (!data || !size) return false;

    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wic));
    if (FAILED(hr)) goto cleanup;

    hr = wic->CreateStream(&stream);
    if (FAILED(hr)) goto cleanup;

    hr = stream->InitializeFromMemory(reinterpret_cast<BYTE*>(data), size);
    if (FAILED(hr)) goto cleanup;

    hr = wic->CreateDecoderFromStream(stream, nullptr, WICDecodeMetadataCacheOnLoad, &decoder);
    if (FAILED(hr)) goto cleanup;

    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) goto cleanup;

    hr = wic->CreateFormatConverter(&converter);
    if (FAILED(hr)) goto cleanup;

    hr = converter->Initialize(frame, GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
    if (FAILED(hr)) goto cleanup;

    // Create Direct2D bitmap into the passed pointer
    hr = g_D2DTarget->CreateBitmapFromWicBitmap(converter, nullptr, ppBitmap);

cleanup:
    SAFE_RELEASE(converter);
    SAFE_RELEASE(frame);
    SAFE_RELEASE(decoder);
    SAFE_RELEASE(stream);
    SAFE_RELEASE(wic);

    return SUCCEEDED(hr) && (*ppBitmap != nullptr);
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

    if (gameStarted && g_Player && g_D2DTarget) {
        bool up = GetAsyncKeyState(VK_UP) & 0x8000;
        bool down = GetAsyncKeyState(VK_DOWN) & 0x8000;
        bool left = GetAsyncKeyState(VK_LEFT) & 0x8000;
        bool right = GetAsyncKeyState(VK_RIGHT) & 0x8000;

        g_Player->HandleInput(up, down, left, right);
        g_Player->Update(dt);

        HandleShooting();

        // Update bullets and remove inactive ones (off-screen)
        for (auto it = g_Bullets.begin(); it != g_Bullets.end(); ) {
            (*it)->Update(dt);
            if (!(*it)->IsActive()) {
                delete* it;
                it = g_Bullets.erase(it); // Remove from list
            }
            else {
                ++it;
            }
        }
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

        if (g_Player)
            g_Player->Render(g_D2DTarget);

        for (Bullet* b : g_Bullets) {
            b->Render(g_D2DTarget);
        }
    }

    g_D2DTarget->EndDraw();

    // REAL VSYNC
    g_SwapChain->Present(1, 0);
}
