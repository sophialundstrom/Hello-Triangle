#include "PipelineHelper.h"
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


bool LoadShaders(ID3D11Device* device, ID3D11VertexShader*& vShader, ID3D11PixelShader*& pShader, std::string& vShaderByteCode)
{
    std::string shaderData;
    std::ifstream reader;

    reader.open("../Debug/VertexShader.cso", std::ios::binary);

    if (!reader.is_open()) 
    {
        std::cerr << "Could not open Vertex Shader" << std::endl;
        return false;
    }

    reader.seekg(0, std::ios::end);
    shaderData.reserve(static_cast<unsigned int>(reader.tellg()));      //Increase capacity to fit in file size
    reader.seekg(0, std::ios::beg);
    shaderData.assign((std::istreambuf_iterator<char>(reader)), std::istreambuf_iterator<char>());      //Assign information to string

    if (FAILED(device->CreateVertexShader(shaderData.c_str(), shaderData.length(), nullptr, &vShader)))
    {
        std::cerr << "Failed to create Vertex Shader" << std::endl;
        return false;
    }

    vShaderByteCode = shaderData;
    shaderData.clear();
    reader.close();

    reader.open("../Debug/PixelShader.cso", std::ios::binary);

    if (!reader.is_open())
    {
        std::cerr << "Could not open Pixel Shader" << std::endl;
        return false;
    }

    reader.seekg(0, std::ios::end);
    shaderData.reserve(static_cast<unsigned int>(reader.tellg()));
    reader.seekg(0, std::ios::beg);
    shaderData.assign((std::istreambuf_iterator<char>(reader)), std::istreambuf_iterator<char>());

    if (FAILED(device->CreatePixelShader(shaderData.c_str(), shaderData.length(), nullptr, &pShader)))
    {
        std::cerr << "Failed to create Pixel Shader" << std::endl;
        return false;
    }

    return true;
}

bool CreateInputLayout(ID3D11Device* device, ID3D11InputLayout*& inputLayout, const std::string& vShaderByteCode) //Describes how vBuffer data will be used into the IA stage
{
    D3D11_INPUT_ELEMENT_DESC inputDesc[3] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},         //32 bit per element -> 1 byte = 8 bit -> 3 * 4 byte = 12
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    //"vShaderByteCode" needed to validate elements (via signature within file)
    HRESULT hr = device->CreateInputLayout(inputDesc, 3, vShaderByteCode.c_str(), vShaderByteCode.length(), &inputLayout);
    return !FAILED(hr);
}

bool CreateVertexBuffer(ID3D11Device* device, ID3D11Buffer*& vBuffer)
{
    D3D11_BUFFER_DESC desc = {};

    VertexData vertices[4] =
    {
        { {-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
        { {-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
        { {0.5, -0.5f, 0.0f} , {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},

        { {0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}}
    };

    desc.ByteWidth = sizeof(vertices);
    desc.Usage = D3D11_USAGE_IMMUTABLE;         //Can only be read by the GPU
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;  
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    
    D3D11_SUBRESOURCE_DATA data = {};           
    data.pSysMem = vertices;

    HRESULT hr = device->CreateBuffer(&desc, &data, &vBuffer);
    return !FAILED(hr);
}

bool CreateConstantBuffer(ID3D11Device* device, ID3D11Buffer*& cBuffer)
{
    int bytes = 128; //(4^3) * 2 (Two 4x4 matrices)

    D3D11_BUFFER_DESC desc = {};
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.Usage = D3D11_USAGE_DYNAMIC;               //GPU read CPU write               
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    desc.ByteWidth = bytes;

    HRESULT hr = device->CreateBuffer(&desc, NULL, &cBuffer);
    return !FAILED(hr);
}

bool CreateTexture(ID3D11Device* device, ID3D11Texture2D*& texture, ID3D11ShaderResourceView*& srv, ID3D11SamplerState*& sampler)
{
    std::string textureImg = "";
    int imgWidth, imgHeight;
    unsigned char* image = stbi_load(textureImg, &imgWidth, &imgHeight, nullptr, STBI_rgb_alpha);
    
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = imgWidth;
    textureDesc.Height = imgHeight;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.MiscFlags = 0;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;    //(R,G,B,A) 8 bit per channel
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    
    D3D11_SUBRESOURCE_DATA data = {};
    data.pSysMem = image;
    data.SysMemPitch = imgWidth * STBI_rgb_alpha;

    if (FAILED(device->CreateTexture2D(&textureDesc, &data, &texture))) {
        std::cerr << "Failed to create Texture2D" << std::endl;
        return false;
    }

    stbi_image_free(image);

    HRESULT hr = device->CreateShaderResourceView(texture, nullptr, &srv);
    
    return !FAILED(hr);
}

bool CreateSamplerState(ID3D11Device* device, ID3D11SamplerState*& sampler)
{
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;          //Keeping quality at different viewing angles
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;      //if outside (1,0), seamless
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

    HRESULT hr = device->CreateSamplerState(&samplerDesc, &sampler);
    return !FAILED(hr);
}

bool CreateLightBuffer(ID3D11Device* device, ID3D11Buffer*& lBuffer)
{
    struct Material 
    {
        float specularPower;
        DirectX::XMFLOAT3 padding;
    };

    struct Light
    {
        DirectX::XMFLOAT4 position;
        DirectX::XMFLOAT4 color;
        DirectX::XMFLOAT3 attenuation;
        float range;
    };

    struct LightMaterialProperties
    {
        Material material;
        Light light;
        DirectX::XMFLOAT4 eyePosition;
    };

    LightMaterialProperties buffer =
    {
        //Material
        {
            150.0f,                     //Specular power
            {0,0,0}                     //Padding
        },

        //Light
        {
            {0.0f, 0.0f, -2.0f, 1.0f},  //Position
            {1.0f, 1.0f, 1.0f, 1.0f},   //Color
            {1.0f, 2/3, 1/9},           //Attenuation
            3.0f                        //Range
        },

        {0.0f, 0.0f, -2.0f, 1.0f}       //Eye position
    };

    D3D11_BUFFER_DESC desc = {};
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.ByteWidth = sizeof(buffer);

    D3D11_SUBRESOURCE_DATA data = {};
    data.pSysMem = &buffer;

    HRESULT hr = device->CreateBuffer(&desc, &data, &lBuffer);
    return !FAILED(hr);
}

void UpdateConstantbuffer(ID3D11Buffer* cBuffer, ID3D11DeviceContext* context, float angle)
{
    struct ConstantBuffer
    {
        DirectX::XMFLOAT4X4 worldViewPerspective;
        DirectX::XMFLOAT4X4 world;
    };

    DirectX::XMFLOAT4X4 worldViewPersp;
    DirectX::XMFLOAT4X4 worldMatrix;

    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f);
    DirectX::XMMATRIX rotY = DirectX::XMMatrixRotationY(angle);
    DirectX::XMMATRIX trans = DirectX::XMMatrixTranslation(0, 0, 1.0f);
    DirectX::XMMATRIX backTrans = DirectX::XMMatrixTranslation(0, 0, -1.0f);
    DirectX::XMMATRIX world = scale * backTrans * rotY * trans;                 //Rotation around (0, 0, 1), world matrix

    DirectX::XMVECTOR eyePos = { 0.0f, 0.0f, -2.0f };
    DirectX::XMVECTOR focus = { 0.0f, 0.0f, 1.0f };
    DirectX::XMVECTOR up = { 0.0f, 1.0f, 0.0f };

    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(eyePos, focus, up);        //View matrix
    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PI * 0.25, (float)1024 / 576, 0.1f, 100.0f);     //Perspective matrix
    DirectX::XMMATRIX WVP = world * viewMatrix * projectionMatrix;

    DirectX::XMStoreFloat4x4(&worldViewPersp, DirectX::XMMatrixTranspose(WVP));
    DirectX::XMStoreFloat4x4(&worldMatrix, DirectX::XMMatrixTranspose(world));

    ConstantBuffer cb
    {
        worldViewPersp,
        worldMatrix
    };

    D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
    context->Map(cBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
    memcpy(mappedBuffer.pData, &cb, sizeof(cb));
    context->Unmap(cBuffer, 0);
}

void BindResourcesToPipeline(ID3D11DeviceContext* context, D3D11_VIEWPORT& viewPort, ID3D11PixelShader* pShader, ID3D11VertexShader* vShader, ID3D11InputLayout* inputLayout, ID3D11ShaderResourceView* srv, ID3D11SamplerState* sampler, ID3D11Buffer* vBuffer, ID3D11Buffer* lBuffer)
{
    UINT stride = sizeof(VertexData);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &vBuffer, &stride, &offset);
    context->IASetInputLayout(inputLayout);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    context->VSSetShader(vShader, nullptr, 0);

    context->PSSetShader(pShader, nullptr, 0);
    context->PSSetShaderResources(0, 1, &srv);
    context->PSSetSamplers(0, 1, &sampler);
    context->PSSetConstantBuffers(0, 1, &lBuffer);
    context->RSSetViewports(1, &viewPort);
}

bool SetupPipeline(ID3D11Device* device, ID3D11Buffer*& vBuffer, ID3D11VertexShader*& vShader,
                   ID3D11PixelShader*& pShader, ID3D11InputLayout*& inputLayout, ID3D11Buffer*& cBuffer,
                   ID3D11Texture2D*& texture, ID3D11ShaderResourceView*& srv, ID3D11SamplerState*& sampler,
                   ID3D11Buffer*& lBuffer)
{
    std::string vShaderByteCode;
    
    if (!LoadShaders(device, vShader, pShader, vShaderByteCode))
    {
        std::cerr << "Failed to load Shaders" << std::endl;
        return false;
    }
    
    if (!CreateInputLayout(device, inputLayout, vShaderByteCode))
    {
        std::cerr << "Failed to create Input Layout" << std::endl;
        return false;
    }

    if (!CreateVertexBuffer(device, vBuffer))
    {
        std::cerr << "Failed to create Vertex Buffer" << std::endl;
        return false;
    }
    
    if (!CreateConstantBuffer(device, cBuffer))
    {
        std::cerr << "Failed to create Constant Buffer" << std::endl;
        return false;
    }

    if (!CreateTexture(device, texture, srv, sampler))
    {
        std::cerr << "Failed to create Texture" << std::endl;
        return false;
    }

    if (!CreateSamplerState(device, sampler))
    {
        std::cerr << "Failed to create Sampler State" << std::endl;
        return false;
    }

    if (!CreateLightBuffer(device, lBuffer))
    {
        std::cerr << "Failed to create Light Buffer" << std::endl;
        return false;
    }

    return true;
}
