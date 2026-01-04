#include "global.h"
#include "ErrorHandler.h"

// ------------------------------------------------------------
// Create Device & SwapChain (REAL VSYNC)
// ------------------------------------------------------------
bool CreateDevice()
{
    // Define initial flags (e.g., for Direct2D compatibility)
    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
    // Add the debug flag only for debug builds in 2026
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    DXGI_SWAP_CHAIN_DESC scd{};
    scd.BufferCount = 2;                                    // Flip models require 2+
    scd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = g_hWnd;                              // Ensure this is not NULL
    scd.SampleDesc.Count = 1;                               // Flip models DO NOT support MSAA
    scd.SampleDesc.Quality = 0;                             // Must be 0
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scd.Flags = 0;                                          // Avoid experimental flags during init

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        featureLevels, ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &scd,
        &g_SwapChain,
        &g_Device,
        nullptr,
        &g_Context);

    if (FAILED(hr))
        ShowHRESULT(g_hWnd, hr, L"D3D11CreateDeviceAndSwapChain() failed");

    // Get the DXGI Factory from your swap chain
    IDXGIFactory* pFactory = nullptr;

    if (SUCCEEDED(g_SwapChain->GetParent(IID_PPV_ARGS(&pFactory))))
    {
        // Tell DXGI to ignore Alt+Enter so we can handle it in WndProc
        pFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER);
        pFactory->Release();
    }

    return SUCCEEDED(hr);
}

// ------------------------------------------------------------
// Create Direct2D Target
// ------------------------------------------------------------
bool CreateD2D()
{
    HRESULT hr = S_OK;

    // 1. Only create factory if it doesn't exist [Fixes your memory leak]
    if (!g_D2DFactory) {
        hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_D2DFactory);
        if (FAILED(hr)) return false;
    }

    // 2. Target MUST be recreated when window sizes change
    IDXGISurface* surface = nullptr;
    hr = g_SwapChain->GetBuffer(0, IID_PPV_ARGS(&surface));
    if (FAILED(hr)) return false;

    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_HARDWARE,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));

    hr = g_D2DFactory->CreateDxgiSurfaceRenderTarget(surface, &props, &g_D2DTarget);
    surface->Release();

    return SUCCEEDED(hr);
}