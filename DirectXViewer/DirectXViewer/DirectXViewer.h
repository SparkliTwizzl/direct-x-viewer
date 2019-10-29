#pragma once

#include <cstdint>
#include <Windows.h>

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")
#include <DirectXMath.h>


using namespace DirectX;


namespace DirectXViewer
{
	// Vertex shader constant buffer data container
	struct CBUFFER_VS_DATA
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	// Pixel shader constant buffer data container
	struct CBUFFER_PS_DATA
	{
		XMFLOAT4 light_pos;
		XMFLOAT3 light_color;
		float light_power;
		float surface_shininess;
		XMFLOAT3 pad;
	};



	// Creates and initializes automatically created D3D and DXV resources
	// Additional resources must be created manually after this function is called
	HRESULT Init(HWND* _hWnd_p);


	// Creates and stores a D3D11 depth stencil and corresponding depth stencil view
	HRESULT DxCreateDepthStencilView(uint32_t _w, uint32_t _h, ID3D11Texture2D** _depthStencil_pp, ID3D11DepthStencilView** _depthStencilView_pp);

	// Creates and stores a D3D11 sampler state
	// DEFAULTS:
	//  _addressU = D3D11_TEXTURE_ADDRESS_WRAP
	//  _addressV = D3D11_TEXTURE_ADDRESS_WRAP
	//  _addressW = D3D11_TEXTURE_ADDRESS_CLAMP (W coord is not used by DirectXViewer)
	//  _comp = D3D11_COMPARISON_NEVER
	//  _minLod = 0.0f
	//  _maxLod = D3D11_FLOAT_MAX
	HRESULT DxCreateSamplerState(
		ID3D11SamplerState** _samplerState_pp, D3D11_FILTER _filter,
		D3D11_TEXTURE_ADDRESS_MODE _addressU = D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_MODE _addressV = D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_COMPARISON_FUNC _comp = D3D11_COMPARISON_NEVER, float _minLod = 0.0f, float _maxLod = D3D11_FLOAT32_MAX);

	// Creates and stores a D3D11 constant buffer
	HRESULT DxCreateConstantBuffer(uint32_t _bytewidth, ID3D11Buffer** _cbuffer_pp);

}
