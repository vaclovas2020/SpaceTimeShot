#include "global.h"
#include "ErrorHandler.h"


// ------------------------------------------------------------
// Load PNG from Resource
// ------------------------------------------------------------
bool LoadPNGFromResource(int resourceId)
{
    IWICImagingFactory* wic = nullptr;
    IWICStream* stream = nullptr;
    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICFormatConverter* converter = nullptr;

    // Locate resource
    HRSRC hrsrc = FindResource(
        g_hInst,
        MAKEINTRESOURCE(resourceId),
        L"PNG");

    if (!hrsrc) return false;

    HGLOBAL hglob = LoadResource(g_hInst, hrsrc);
    if (!hglob) return false;

    void* data = LockResource(hglob);
    DWORD size = SizeofResource(g_hInst, hrsrc);
    if (!data || !size) return false;

    // Create WIC factory
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&wic));

    if (FAILED(hr))
    {
        ShowHRESULT(g_hWnd, hr, L"CoCreateInstance() failed");

        goto cleanup;
    }

    // Create stream from memory
    hr = wic->CreateStream(&stream);

    if (FAILED(hr))
    {
        ShowHRESULT(g_hWnd, hr, L"CreateStream() failed");

        goto cleanup;
    }

    hr = stream->InitializeFromMemory(
        reinterpret_cast<BYTE*>(data),
        size);

    if (FAILED(hr))
    {
        ShowHRESULT(g_hWnd, hr, L"InitializeFromMemory() failed");

        goto cleanup;
    }

    // Decode PNG
    hr = wic->CreateDecoderFromStream(
        stream,
        nullptr,
        WICDecodeMetadataCacheOnLoad,
        &decoder);

    if (FAILED(hr))
    {
        ShowHRESULT(g_hWnd, hr, L"CreateDecoderFromStream() failed");

        goto cleanup;
    }

    hr = decoder->GetFrame(0, &frame);

    if (FAILED(hr))
    {
        ShowHRESULT(g_hWnd, hr, L"decoder->GetFrame() failed");

        goto cleanup;
    }

    // Convert to 32bpp premultiplied BGRA
    hr = wic->CreateFormatConverter(&converter);

    if (FAILED(hr))
    {
        ShowHRESULT(g_hWnd, hr, L"wic->CreateFormatConverter() failed");

        goto cleanup;
    }

    hr = converter->Initialize(
        frame,
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeCustom);

    if (FAILED(hr))
    {
        ShowHRESULT(g_hWnd, hr, L"converter->Initialize() failed");

        goto cleanup;
    }

    // Create Direct2D bitmap
    hr = g_D2DTarget->CreateBitmapFromWicBitmap(
        converter,
        nullptr,
        &g_BackgroundBitmap);

    if (FAILED(hr))
    {
        ShowHRESULT(g_hWnd, hr, L"g_D2DTarget->CreateBitmapFromWicBitmap() failed");

        goto cleanup;
    }

cleanup:
    SAFE_RELEASE(converter);
    SAFE_RELEASE(frame);
    SAFE_RELEASE(decoder);
    SAFE_RELEASE(stream);
    SAFE_RELEASE(wic);

    return SUCCEEDED(hr) && g_BackgroundBitmap != nullptr;
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
    }

    g_D2DTarget->EndDraw();

    // REAL VSYNC
    g_SwapChain->Present(1, 0);
}
