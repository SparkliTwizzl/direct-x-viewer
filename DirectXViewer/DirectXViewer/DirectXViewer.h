#pragma once

#include <cstdint>
#include <Windows.h>

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")
#include <DirectXMath.h>


using namespace DirectX;


namespace DirectXViewer
{
	// DXV vertex data container
	struct DXVVERTEX
	{
		XMFLOAT3 pos;
		XMFLOAT3 norm;
		XMFLOAT4 color;
		XMFLOAT2 uv;
	};

	// DXV mesh data container
	struct DXVMESHDATA
	{
		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;
		DXVVERTEX* vertices = nullptr;
		uint32_t* indices = nullptr;
	};

	// DXV rendering mesh
	// NOTES:
	//  Must be contained in a DXVOBJECT along with a DXVMATERIAL
	struct DXVMESH
	{
		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;
		ID3D11Buffer* vertexBuffer_p = nullptr;
		ID3D11Buffer* indexBuffer_p = nullptr;
	};

	// DXV material data container
	struct DXVMATERIALDATA
	{

	};

	// DXV rendering material
	// NOTES:
	//  Must be contained in a DXVOBJECT along with a DXVMESH
	struct DXVMATERIAL
	{

	};

	// DXV rendering object
	struct DXVOBJECT
	{
		DXVMESH* mesh = nullptr;
		DXVMATERIAL* material = nullptr;
	};

	// Vertex shader constant buffer data container
	struct DXVCBUFFER_VS
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	// Pixel shader constant buffer data container
	struct DXVCBUFFER_PS
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

	// Handles updates for automatically created D3D and DXV resources
	// Additional updates must be done manually after this function is called
	void Update();

	// Handles drawing of active DXV resources
	// Additional draws must be done manually after this function is called
	void Draw();

	// Frees memory used by automatically created D3D and DXV resources
	// Additional heap memory must be cleared manually before this function is called
	void Cleanup();


	// Returns the world matrix
	XMMATRIX GetWorldMatrix();

	// Returns the view matrix
	XMMATRIX GetViewMatrix();

	// Returns the projection matrix
	XMMATRIX GetProjectionMatrix();


	// Sets the world matrix
	void SetWorldMatrix(XMMATRIX _m);

	// Sets the view matrix
	void SetViewMatrix(XMMATRIX _m);

	// Sets the projection matrix
	void SetProjectionMatrix(XMMATRIX _m);


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

	// Initializes a D3D11 viewport
	// DEFAULTS:
	//  _topLeftX = 0
	//  _topLeftY = 0
	//  _minDepth = 0.0f
	//  _maxDepth = 1.0f
	void DxSetupViewport(D3D11_VIEWPORT* _viewport_p, float _w, float _h, float _topLeftX = 0.0f, float _topLeftY = 0.0f, float _minDepth = 0.0f, float _maxDepth = 1.0f);

}
