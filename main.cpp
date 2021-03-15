#include <Windows.h>
#include <iostream>
#include <d3d11.h>
#include <chrono>

#include "WindowHelper.h"
#include "D3D11Handler.h"
#include "PipelineHelper.h"

struct Timer
{
	std::chrono::time_point<std::chrono::steady_clock> start, end;

	void startTimer()
	{
		start = std::chrono::steady_clock::now();
	}

	double deltaTime()
	{
		end = std::chrono::steady_clock::now();
		return (double)(end - start).count();
	}
};

void Render(float* backgroundColor, ID3D11DeviceContext* context, ID3D11RenderTargetView* rtv, 
	ID3D11DepthStencilView* dsView, ID3D11Buffer* cBuffer, float angle)
{
	UpdateConstantbuffer(cBuffer, context, angle);

	context->ClearRenderTargetView(rtv, backgroundColor);
	context->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

	context->VSSetConstantBuffers(0, 1, &cBuffer);

	context->OMSetRenderTargets(1, &rtv, dsView);

	context->Draw(4, 0);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	const UINT WIDTH = 1024;
	const UINT HEIGHT = 576;

	float backgroundColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	double rotation = 0.000000001;

	HWND window;

	ID3D11Device* device;			//Creates resources
	ID3D11DeviceContext* context;	//Contains the bindings to pipeline & rendering commands
	IDXGISwapChain* swapChain;		//implements surfaces for storing rendered data before presenting
	ID3D11RenderTargetView* rtv;	//identifies render-target subresources during rendering
	ID3D11Texture2D* dsTexture;		//depth-stencil texture, used by depthStencilView to decide culling
	ID3D11DepthStencilView* dsView;	//accesing the texture for depth-stencil testing (culling)
	D3D11_VIEWPORT viewPort;		//defining dimensions & settings for a viewport
	ID3D11VertexShader* vShader;	//per-vertex
	ID3D11PixelShader* pShader;		//per-fragment
	ID3D11InputLayout* inputLayout;	//how IA-stage will read vertex data
	ID3D11Buffer* vBuffer;			//vertex data
	ID3D11Buffer* cBuffer;			//cbuffer data (to vertex shader)
	ID3D11Texture2D* texture;		//image texture
	ID3D11ShaderResourceView* srv;	//specifies the subreasources the pixel shader can access
	ID3D11SamplerState* sampler;	//needed to be able to sample from texture (in pixel shader)
	ID3D11Buffer* lBuffer;			//cbuffer data (to pixel shader)
	
	if (!SetupWindow(hInstance, WIDTH, HEIGHT, nCmdShow, window)) 
	{
		std::cerr << "failed to setup window" << std::endl;
		return -1;
	}

	if (!SetupD3D11(WIDTH, HEIGHT, window, device, context, swapChain, rtv, dsTexture, dsView, viewPort)) 
	{
		std::cerr << "Could not setup d3d11" << std::endl;
		return -1;
	}

	if (!SetupPipeline(device, vBuffer, vShader, pShader, inputLayout, cBuffer, texture, srv, sampler, lBuffer))
	{
		std::cerr << "Could not setup Pipeline" << std::endl;
		return -1;
	}

	BindResourcesToPipeline(context, viewPort, pShader, vShader, inputLayout, srv, sampler, vBuffer, lBuffer);

	MSG msg = {};
	float angle = 0;
	Timer timer;

	while (msg.message != WM_QUIT) 
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		timer.startTimer();

		Render(backgroundColor, context, rtv, dsView, cBuffer, angle);
		swapChain->Present(0, 0);

		angle += float(rotation * timer.deltaTime());
		if (angle >= DirectX::XM_2PI)
			angle = 0;
	}
	
	lBuffer->Release();
	sampler->Release();
	srv->Release();
	texture->Release();
	cBuffer->Release();
	vBuffer->Release();
	inputLayout->Release();
	pShader->Release();
	vShader->Release();
	dsView->Release();
	dsTexture->Release();
	rtv->Release();
	swapChain->Release();
	context->Release();
	device->Release();
	
	return 0;
}