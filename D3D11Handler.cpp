#include "D3D11Handler.h"

bool CreateInterface(UINT winWidth, UINT winHeight, HWND window, ID3D11Device*& device, ID3D11DeviceContext*& context, IDXGISwapChain*& swapChain) 
{
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

    swapChainDesc.BufferDesc.Width = winWidth;
    swapChainDesc.BufferDesc.Height = winHeight;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;         //Render as fast as possible (no set refresh rate)
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;      //32 Bit,  Unsigned normalized int, clamping values between 0.0f and 1.0f, eg. 11111111 = 1.0f

    swapChainDesc.SampleDesc.Count = 1;     //Can changed to enable anti-aliasing and blurring
    swapChainDesc.SampleDesc.Quality = 0;   //Default values, performance friendly
    
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;    //Using resource as output render target
    swapChainDesc.BufferCount = 1;
    swapChainDesc.OutputWindow = window;
    swapChainDesc.Windowed = true;
    swapChainDesc.Flags = 0;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;    //Clear memory after rendering

    UINT flags = 0;

    if (_DEBUG)
        flags = D3D11_CREATE_DEVICE_DEBUG;

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };     //Minimum hardware requirement

    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels, 1,                 //Adapter null, default adapter
                                               D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, nullptr, &context);          //Software null because we are using driver type hardware
                                                                                                                                    //Last null, we dont need to know which feature level is supported (assuming)
    return !FAILED(hr);
}

bool CreateRTV(ID3D11Device* device, IDXGISwapChain* swapChain, ID3D11RenderTargetView*& rtv) 
{
    ID3D11Texture2D* backBuffer = {};

    if (FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)))) {
        std::cerr << "Failed to get back buffer" << std::endl;
        return false;
    }
    
    HRESULT hr = device->CreateRenderTargetView(backBuffer, nullptr, &rtv);
    backBuffer->Release();

    return !FAILED(hr);
}

bool CreateDepthStencil(ID3D11Device* device, UINT winWidth, UINT winHeight, ID3D11Texture2D*& dsTexture, ID3D11DepthStencilView*& dsView)
{
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = winWidth;
    textureDesc.Height = winHeight;
    textureDesc.MipLevels = 1;                              //Used for mip-mapping, default value 1
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;     //24 bit to depth buffer, 8 bit to stencil buffer
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;                //GPU read & write accessibility
    textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;       //Bind as depth-stencil
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

    if (FAILED(device->CreateTexture2D(&textureDesc, nullptr, &dsTexture))) {       
        std::cerr << "Failed to create depth stencil texture" << std::endl;
        return false;
    }
    
    HRESULT hr = device->CreateDepthStencilView(dsTexture, nullptr, &dsView);       //Culling

    return !FAILED(hr);
}

void SetViewPort(D3D11_VIEWPORT& viewPort, UINT winWidth, UINT winHeight)
{
    viewPort.Width = static_cast<float>(winWidth);
    viewPort.Height = static_cast<float>(winHeight);
    viewPort.TopLeftX = 0;
    viewPort.TopLeftY = 0;
    viewPort.MinDepth = 0;
    viewPort.MaxDepth = 1;
}

bool SetupD3D11(UINT winWidth, UINT winHeight, HWND window, ID3D11Device*& device, ID3D11DeviceContext*& context, IDXGISwapChain*& swapChain, 
                ID3D11RenderTargetView*& rtv, ID3D11Texture2D*& dsTexture, ID3D11DepthStencilView*& dsView, D3D11_VIEWPORT& viewPort)
{
    if (!CreateInterface(winWidth, winHeight, window, device, context, swapChain)) {
        std::cerr << "Could not create interface" << std::endl;
        return false;
    }
    
    if (!CreateRTV(device, swapChain, rtv)) {
        std::cerr << "Could not create render taget view" << std::endl;
        return false;
    }

    if (!CreateDepthStencil(device, winWidth, winHeight, dsTexture, dsView)) {
        std::cerr << "Could not create Depth Stencil" << std::endl;
        return false;
    }

    SetViewPort(viewPort, winWidth, winHeight);

    return true;
}
