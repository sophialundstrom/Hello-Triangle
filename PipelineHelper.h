#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <iostream>
#include <fstream>
#include <string>
#include <array>

struct VertexData 
{
	float pos[3];
	float nor[3];
	float uv[2];

	VertexData(const std::array<float, 3>& position, const std::array<float, 3>& normal, const std::array<float, 2>& uvCoords) 
	{
		for (int i = 0; i < 3; ++i) 
		{
			pos[i] = position[i];
			nor[i] = normal[i];
			
		}

		uv[0] = uvCoords[0];
		uv[1] = uvCoords[1];
	}
};

void UpdateConstantbuffer(ID3D11Buffer* cBuffer, ID3D11DeviceContext* context, float angle);

void BindResourcesToPipeline(ID3D11DeviceContext* context, D3D11_VIEWPORT& viewPort, ID3D11PixelShader* pShader, ID3D11VertexShader* vShader, ID3D11InputLayout* inputLayout, ID3D11ShaderResourceView* srv, ID3D11SamplerState* sampler, ID3D11Buffer* vBuffer, ID3D11Buffer* lBuffer);

bool SetupPipeline(ID3D11Device* device, ID3D11Buffer*& vBuffer, ID3D11VertexShader*& vShader,
	ID3D11PixelShader*& pShader, ID3D11InputLayout*& inputLayout, ID3D11Buffer*& cBuffer, ID3D11Texture2D*& texture,
	ID3D11ShaderResourceView*& srv, ID3D11SamplerState*& sampler, ID3D11Buffer*& lBuffer);