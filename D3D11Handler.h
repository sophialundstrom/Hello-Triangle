#pragma once

#include <Windows.h>
#include <iostream>
#include <d3d11.h>

bool SetupD3D11(UINT winWidth, UINT winHeight, HWND window, ID3D11Device*& device, ID3D11DeviceContext*& context, 
				IDXGISwapChain*& swapChain, ID3D11RenderTargetView*& rtv, ID3D11Texture2D*& dsTexture, 
				ID3D11DepthStencilView*& dsView, D3D11_VIEWPORT& viewPort);
