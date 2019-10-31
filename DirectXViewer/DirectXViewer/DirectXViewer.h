#pragma once

#include <cstdint>
#include <Windows.h>

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")
#include <DirectXMath.h>

#include "colors.h"


using namespace DirectX;


namespace DirectXViewer
{
#pragma region Defines
#define IBUFFER_FORMAT DXGI_FORMAT_R32_UINT
#pragma endregion


#pragma region Structs
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

		~DXVMESHDATA()
		{
			delete[] vertices;
			delete[] indices;
		}
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

		~DXVMESH()
		{
			if (indexBuffer_p) indexBuffer_p->Release();
			if (vertexBuffer_p) vertexBuffer_p->Release();
		}
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
#pragma endregion


#pragma region Basic Functions
	// Creates and initializes automatically created D3D and DXV resources
	// Additional resources must be created manually after this function is called
	HRESULT Init(HWND* _hWnd_p);

	// Handles updates for automatically created D3D and DXV resources
	// Additional updates must be done manually after this function is called
	void Update(const MSG* _msg);

	// Handles drawing of active DXV resources
	// Additional draws must be done manually after this function is called
	// NOTES:
	//  Pass false if you are manually drawing anything, then call Present() after manual draws
	void Draw(bool _present = true);

	// Presents the scene to the window
	// Only needed if you are drawing anything manually
	void Present(UINT _syncInterval = 1, UINT _flags = 0);

	// Frees memory used by automatically created D3D and DXV resources
	// Additional heap memory must be cleared manually before this function is called
	void Cleanup();
#pragma endregion

#pragma region Getters/Setters
	// Returns the world matrix
	XMMATRIX GetWorldMatrix();

	// Returns the view matrix
	XMMATRIX GetViewMatrix();

	// Returns the projection matrix
	XMMATRIX GetProjectionMatrix();

	// Returns a pointer to the device
	ID3D11Device* GetDevice();

	// Returns a pointer to the device context
	ID3D11DeviceContext* GetDeviceContext();

	// Returns a pointer to the swap chain
	IDXGISwapChain* GetSwapChain();

	// Returns the last error message generated
	const char* GetLastError();


	// Sets the world matrix
	void SetWorldMatrix(XMMATRIX _m);

	// Sets the view matrix
	void SetViewMatrix(XMMATRIX _m);

	// Sets the projection matrix
	void SetProjectionMatrix(XMMATRIX _m);
#pragma endregion

#pragma region Mesh/Material Functions
	// Loads mesh data from file into a DXVMESHDATA
	HRESULT DXVLoadMeshData(const char* _filepath, DXVMESHDATA** _meshdata_pp);

	// Creates and stores a DXVMESH from a DXVMESHDATA
	HRESULT DXVCreateMesh(DXVMESHDATA* _meshdata_p, DXVMESH** _mesh_pp);
#pragma endregion

#pragma region Scene Functions
	// Adds an object to the scene
	void AddObjectToScene(DXVOBJECT* _obj_p);

	// Returns the specified object from the scene if index is valid
	DXVOBJECT* GetObjectFromScene(uint16_t _i);

	// Checks the scene for the object specified and removes it if found
	void RemoveObjectFromScene(DXVOBJECT* _obj_p);
#pragma endregion

#pragma region DirectX Helper Functions
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

	// Creates and stores a D3D11 vertex buffer from a DXV vertex array
	HRESULT DxCreateVertexBuffer(uint32_t _vertCount, DXVVERTEX* _verts, ID3D11Buffer** _vbuffer_pp);

	// Creates and stores a D3D11 index buffer from a DXV vertex array
	HRESULT DxCreateIndexBuffer(uint32_t _indCount, uint32_t* _inds, ID3D11Buffer** _ibuffer_pp);

	// Initializes a D3D11 viewport
	// DEFAULTS:
	//  _topLeftX = 0
	//  _topLeftY = 0
	//  _minDepth = 0.0f
	//  _maxDepth = 1.0f
	void DxSetupViewport(D3D11_VIEWPORT* _viewport_p, float _w, float _h, float _topLeftX = 0.0f, float _topLeftY = 0.0f, float _minDepth = 0.0f, float _maxDepth = 1.0f);
#pragma endregion

}
