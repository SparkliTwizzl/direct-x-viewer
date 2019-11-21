#include <fstream>
#include <vector>

#include"WICTextureLoader.h"

#include "vertexshader_default.csh"
#include "pixelshader_default.csh"
#include "pixelshader_debug.csh"

#include "DirectXViewer.h"


namespace DirectXViewer
{
#pragma region Window Variables
	HWND*							hWnd_p = nullptr;

	uint32_t						windowWidth = 0;
	uint32_t						windowHeight = 0;
#pragma endregion

#pragma region D3D Variables
	D3D_DRIVER_TYPE					driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL				featureLevel = D3D_FEATURE_LEVEL_11_0;

	ID3D11Device*					device_p = nullptr;
	ID3D11DeviceContext*			deviceContext_p = nullptr;
	IDXGISwapChain*					swapChain_p = nullptr;

	ID3D11RenderTargetView*			renderTargetViews[RENDER_TARGET_VIEW::COUNT];
	ID3D11Texture2D*				depthStencils[DEPTH_STENCIL::COUNT];
	ID3D11DepthStencilView*			depthStencilViews[DEPTH_STENCIL_VIEW::COUNT];

	ID3D11InputLayout*				vertexLayouts[VERTEX_LAYOUT::COUNT];

	ID3D11SamplerState*				samplerStates[SAMPLER_STATE::COUNT];

	ID3D11Buffer*					vertexBuffers[VERTEX_BUFFER::COUNT];

	DXVCBUFFER_VS					constantBuffers_vs[CONSTANT_BUFFER_VS::COUNT];
	DXVCBUFFER_PS					constantBuffers_ps[CONSTANT_BUFFER_VS::COUNT];

	ID3D11Buffer*					constantBuffers_vs_D3D[CONSTANT_BUFFER_VS::COUNT];
	ID3D11Buffer*					constantBuffers_ps_D3D[CONSTANT_BUFFER_VS::COUNT];

	ID3D11VertexShader*				shaders_vertex[SHADER_VERTEX::COUNT];
	ID3D11PixelShader*				shaders_pixel[SHADER_PIXEL::COUNT];

	D3D11_VIEWPORT					viewports[VIEWPORT::COUNT];

	uint32_t						strides[] = { sizeof(DXVVERTEX) };
	uint32_t						offsets[] = { 0 };
#pragma endregion

#pragma region DXV Variables
	const char*						errormsg;

	XMFLOAT4X4						world;
	XMFLOAT4X4						view;
	XMFLOAT4X4						projection;

	float							clearColor[4] = BLACK_RGBA_FLOAT;

	XMFLOAT3						light_pos = { 0.0f, 5.0f, 5.0f };
	XMFLOAT3						light_color = WHITE_RGB_FLOAT;
	float							light_power = 10.0f;
	float							light_rotationSpeed = 2.0f;

	float							surface_shininess = 10.0f;

	std::vector<DXVOBJECT*>			sceneObjects;
#pragma endregion


#pragma region Private Helper Functions
	// setters

	// sets the inverse-transpose world matrix (used for normal vectors in shaders)
	void inline _SetWorldITMatrix(XMMATRIX _m) { constantBuffers_vs[CONSTANT_BUFFER_VS::DEFAULT].worldIT = XMMatrixTranspose(XMMatrixInverse(nullptr, _m)); }


	// init

	HRESULT _CreateDeviceAndSwapChain()
	{
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferCount = 1; // number of buffers in swap chain
		swapChainDesc.OutputWindow = *hWnd_p;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // pixel format
		swapChainDesc.BufferDesc.Width = windowWidth;
		swapChainDesc.BufferDesc.Height = windowHeight;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // buffer usage flag; specifies what swap chain's buffer will be used for
		swapChainDesc.SampleDesc.Count = 1; // samples per pixel while drawing
		UINT createDeviceFlags = 0;
#ifdef _DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		return D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
			createDeviceFlags, &featureLevel, 1, D3D11_SDK_VERSION,
			&swapChainDesc, &swapChain_p, &device_p, 0, &deviceContext_p);
	}
	HRESULT _CreateRenderTargetViews()
	{
		HRESULT hr;

		// get back buffer from swap chain and create render target view
		ID3D11Resource* backbuffer_p = nullptr;
		hr = swapChain_p->GetBuffer(0, __uuidof(backbuffer_p), (void**)&backbuffer_p);
		if (FAILED(hr)) return hr;

		hr = device_p->CreateRenderTargetView(backbuffer_p, nullptr, &renderTargetViews[RENDER_TARGET_VIEW::DEFAULT]);
		if (FAILED(hr)) return hr;

		backbuffer_p->Release();

		return hr;
	}
	HRESULT _CreateDepthStencils()
	{
		HRESULT hr;

		hr = D3DCreateDepthStencilView(windowWidth, windowHeight, &depthStencils[DEPTH_STENCIL::DEFAULT], &depthStencilViews[DEPTH_STENCIL_VIEW::DEFAULT]);
		if (FAILED(hr)) return hr;

		return hr;
	}
	HRESULT _CreateInputLayouts()
	{
		HRESULT hr;

		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		UINT numInputElements = ARRAYSIZE(inputElementDesc);
		hr = device_p->CreateInputLayout(inputElementDesc, numInputElements, vertexshader_default, sizeof(vertexshader_default), &vertexLayouts[VERTEX_LAYOUT::DEFAULT]);

		return hr;
	}
	HRESULT _CreateSamplerStates()
	{
		HRESULT hr;

		hr = D3DCreateSamplerState(&samplerStates[SAMPLER_STATE::LINEAR], D3D11_FILTER_MIN_MAG_MIP_LINEAR);
		if (FAILED(hr)) return hr;

		return hr;
	}
	HRESULT _CreateVertexBuffers()
	{
		HRESULT hr;

		hr = D3DCreateVertexBuffer((uint32_t)DebugRenderer::get_line_vert_capacity(), (DXVVERTEX*)DebugRenderer::get_line_verts(), &vertexBuffers[VERTEX_BUFFER::DEBUG]);
		if (FAILED(hr)) return hr;

		return hr;
	}
	HRESULT _CreateConstantBuffers()
	{
		HRESULT hr;

		hr = D3DCreateConstantBuffer(sizeof(DXVCBUFFER_VS), &constantBuffers_vs_D3D[CONSTANT_BUFFER_VS::DEFAULT]);
		if (FAILED(hr)) return hr;

		hr = D3DCreateConstantBuffer(sizeof(DXVCBUFFER_PS), &constantBuffers_ps_D3D[CONSTANT_BUFFER_PS::DEFAULT]);
		if (FAILED(hr)) return hr;

		return hr;
	}
	HRESULT _CreateShaders()
	{
		HRESULT hr;

		hr = device_p->CreateVertexShader(vertexshader_default, sizeof(vertexshader_default), nullptr, &shaders_vertex[SHADER_VERTEX::DEFAULT]);
		if (FAILED(hr)) return hr;

		hr = device_p->CreatePixelShader(pixelshader_default, sizeof(pixelshader_default), nullptr, &shaders_pixel[SHADER_PIXEL::DEFAULT]);
		if (FAILED(hr)) return hr;

		hr = device_p->CreatePixelShader(pixelshader_debug, sizeof(pixelshader_debug), nullptr, &shaders_pixel[SHADER_PIXEL::DEBUG]);
		if (FAILED(hr)) return hr;

		return hr;
	}

	HRESULT	_InitD3DResources()
	{
		HRESULT hr;

		errormsg = "D3D initialization failed";


		hr = _CreateDeviceAndSwapChain();
		if (FAILED(hr)) return hr;

		hr = _CreateRenderTargetViews();
		if (FAILED(hr)) return hr;

		hr = _CreateDepthStencils();
		if (FAILED(hr)) return hr;

		hr = _CreateInputLayouts();
		if (FAILED(hr)) return hr;

		deviceContext_p->IASetInputLayout(vertexLayouts[VERTEX_LAYOUT::DEFAULT]);

		hr = _CreateSamplerStates();
		if (FAILED(hr)) return hr;

		hr = _CreateVertexBuffers();
		if (FAILED(hr)) return hr;

		hr = _CreateConstantBuffers();
		if (FAILED(hr)) return hr;

		hr = _CreateShaders();
		if (FAILED(hr)) return hr;

		D3DConfigureViewport(&viewports[VIEWPORT::DEFAULT], (float)windowWidth, (float)windowHeight);

		deviceContext_p->IASetPrimitiveTopology(DXV_PRIMITIVE_TOPOLOGY_DEFAULT);


		return hr;
	}
	HRESULT _OpenFile(const char* _filepath, std::fstream* _fin)
	{
		HRESULT hr = E_INVALIDARG;

		*_fin = std::fstream(_filepath, std::ios_base::in | std::ios_base::binary);

		if (_fin->is_open())
		{
			hr = S_OK;
		}
		else
		{
			errormsg = "Could not open file for reading";
		}

		return hr;
	}
#pragma endregion

#pragma region Getters
	const char* GetLastError() { return errormsg; }

	XMMATRIX GetDefaultWorldMatrix() { return XMLoadFloat4x4(&world); }
	XMMATRIX GetDefaultViewMatrix() { return XMLoadFloat4x4(&view); }
	XMMATRIX GetDefaultProjectionMatrix() { return XMLoadFloat4x4(&projection); }
	XMMATRIX GetCurrentWorldMatrix() { return constantBuffers_vs[CONSTANT_BUFFER_VS::DEFAULT].world; }
	XMMATRIX GetCurrentViewMatrix() { return constantBuffers_vs[CONSTANT_BUFFER_VS::DEFAULT].view; }
	XMMATRIX GetCurrentProjectionMatrix() { return constantBuffers_vs[CONSTANT_BUFFER_VS::DEFAULT].projection; }

	ID3D11Device* GetDevice() { return device_p; }
	ID3D11DeviceContext* GetDeviceContext() { return deviceContext_p; }
	IDXGISwapChain* GetSwapChain() { return swapChain_p; }
#pragma endregion

#pragma region Setters
	void SetDefaultWorldMatrix(XMMATRIX _m) { XMStoreFloat4x4(&world, _m); }
	void SetDefaultViewMatrix(XMMATRIX _m) { XMStoreFloat4x4(&view, _m); }
	void SetDefaultProjectionMatrix(XMMATRIX _m) { XMStoreFloat4x4(&projection, _m); }
	void SetCurrentWorldMatrix(XMMATRIX _m)
	{
		constantBuffers_vs[CONSTANT_BUFFER_VS::DEFAULT].world = _m;
		_SetWorldITMatrix(_m);
	}
	void SetCurrentViewMatrix(XMMATRIX _m) { constantBuffers_vs[CONSTANT_BUFFER_VS::DEFAULT].view = _m; }
	void SetCurrentProjectionMatrix(XMMATRIX _m) { constantBuffers_vs[CONSTANT_BUFFER_VS::DEFAULT].projection = _m; }

	void SetSurfaceShininess(float _s) { constantBuffers_ps[CONSTANT_BUFFER_VS::DEFAULT].surface_shininess = _s; }

	void D3DSetVertexBuffer(ID3D11Buffer** _vbuffer_pp) { deviceContext_p->IASetVertexBuffers(0, 1, _vbuffer_pp, strides, offsets); }
	void D3DSetIndexBuffer(ID3D11Buffer* _ibuffer_p) { deviceContext_p->IASetIndexBuffer(_ibuffer_p, IBUFFER_FORMAT, 0); }
	void D3DSetDiffuseMaterial(ID3D11ShaderResourceView* _material_p) { deviceContext_p->PSSetShaderResources(0, 1, &_material_p); }
	void D3DSetEmissiveMaterial(ID3D11ShaderResourceView* _material_p) { deviceContext_p->PSSetShaderResources(1, 1, &_material_p); }
	void D3DSetSpecularMaterial(ID3D11ShaderResourceView* _material_p) { deviceContext_p->PSSetShaderResources(2, 1, &_material_p); }
	void D3DSetNormalMapMaterial(ID3D11ShaderResourceView* _material_p) { deviceContext_p->PSSetShaderResources(3, 1, &_material_p); }
	void D3DSetClearColor(XMFLOAT4 _color)
	{
		clearColor[0] = _color.x;
		clearColor[1] = _color.y;
		clearColor[2] = _color.z;
		clearColor[3] = _color.w;
	}

	void DXVSetMesh(DXVMESH* _mesh_p)
	{
		D3DSetVertexBuffer(&_mesh_p->vertexBuffer_p);
		D3DSetIndexBuffer(_mesh_p->indexBuffer_p);
	}
	void DXVSetMaterial(DXVMATERIAL* _material_p)
	{
		D3DSetDiffuseMaterial(_material_p->components[DXVMATERIAL::ComponentType_e::Diffuse].textureView_p);
		D3DSetEmissiveMaterial(_material_p->components[DXVMATERIAL::ComponentType_e::Emissive].textureView_p);
		D3DSetSpecularMaterial(_material_p->components[DXVMATERIAL::ComponentType_e::Specular].textureView_p);
		D3DSetNormalMapMaterial(_material_p->components[DXVMATERIAL::ComponentType_e::NormalMap].textureView_p);
	}
	void DXVSetObject(DXVOBJECT* _object_p)
	{
		SetCurrentWorldMatrix(_object_p->model_matrix);
		DXVSetMesh(_object_p->mesh_p);
		DXVSetMaterial(_object_p->material_p);

		UpdateVSConstantBuffer();
	}
#pragma endregion

#pragma region Basic Functions
	HRESULT Init(HWND* _hWnd_p)
	{
		HRESULT hr;

		hWnd_p = _hWnd_p;

		// get window dimensions
		RECT wRect;
		GetClientRect(*hWnd_p, &wRect);
		windowWidth = wRect.right - wRect.left;
		windowHeight = wRect.bottom - wRect.top;


		// initialize basic D3D resources
		hr = _InitD3DResources();
		if (FAILED(hr)) return hr;


		errormsg = "Initialization error";


		// initialize matrix values
		SetDefaultWorldMatrix(XMMatrixIdentity());

		XMVECTOR eye = { 0, 0, -5 };
		XMVECTOR at = { 0, 0, 0 };
		XMVECTOR up = { 0, 1, 0 };
		SetDefaultViewMatrix(XMMatrixLookAtLH(eye, at, up));

		SetDefaultProjectionMatrix(XMMatrixPerspectiveFovLH(XM_PIDIV4, windowWidth / (float)windowHeight, 0.01f, 100.0f));


		// initialize WIC texture loader
		hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
		if (FAILED(hr)) return hr;


		// clear error message
		errormsg = "";


		return hr;
	}
	void Update(const MSG* _msg, float _dt)
	{
		DebugRenderer::clear_lines();

		// orbit light around origin
		XMVECTOR t_light_pos = { light_pos.x, light_pos.y, light_pos.z };
		t_light_pos = (XMMatrixTranslationFromVector(t_light_pos) * XMMatrixRotationY(light_rotationSpeed * _dt)).r[3];
		light_pos = { XMVectorGetX(t_light_pos), XMVectorGetY(t_light_pos), XMVectorGetZ(t_light_pos) };
	}
	void Cleanup()
	{
		// release D3D resources
#define RELEASE(p) if (p) p->Release()

		uint32_t i;

		for (i = 0; i < SHADER_PIXEL::COUNT; i++)
			RELEASE(shaders_pixel[i]);

		for (i = 0; i < SHADER_VERTEX::COUNT; i++)
			RELEASE(shaders_vertex[i]);

		for (i = 0; i < CONSTANT_BUFFER_PS::COUNT; i++)
			RELEASE(constantBuffers_ps_D3D[i]);

		for (i = 0; i < CONSTANT_BUFFER_VS::COUNT; i++)
			RELEASE(constantBuffers_vs_D3D[i]);

		for (i = 0; i < VERTEX_BUFFER::COUNT; i++)
			RELEASE(vertexBuffers[i]);

		for (i = 0; i < SAMPLER_STATE::COUNT; i++)
			RELEASE(samplerStates[i]);

		for (i = 0; i < VERTEX_LAYOUT::COUNT; i++)
			RELEASE(vertexLayouts[i]);

		for (i = 0; i < DEPTH_STENCIL_VIEW::COUNT; i++)
			RELEASE(depthStencilViews[i]);

		for (i = 0; i < DEPTH_STENCIL::COUNT; i++)
			RELEASE(depthStencils[i]);

		for (i = 0; i < RENDER_TARGET_VIEW::COUNT; i++)
			RELEASE(renderTargetViews[i]);

		RELEASE(swapChain_p);
		RELEASE(deviceContext_p);
		RELEASE(device_p);

#undef RELEASE

		// remove objects from scene
		sceneObjects.clear();

		// uninitialize WIC texture loader
		CoUninitialize();
	}
#pragma endregion

#pragma region Draw Functions
	void Draw(uint32_t _draw_mode)
	{
		// clear constant buffer data
		constantBuffers_vs[CONSTANT_BUFFER_VS::DEFAULT] = {};
		constantBuffers_ps[CONSTANT_BUFFER_PS::DEFAULT] = {};

		// set viewport
		deviceContext_p->RSSetViewports(1, &viewports[VIEWPORT::DEFAULT]);

		// set and clear render target and depth stencil
		deviceContext_p->OMSetRenderTargets(1, &renderTargetViews[RENDER_TARGET_VIEW::DEFAULT], depthStencilViews[DEPTH_STENCIL_VIEW::DEFAULT]);
		deviceContext_p->ClearRenderTargetView(renderTargetViews[RENDER_TARGET_VIEW::DEFAULT], clearColor);
		deviceContext_p->ClearDepthStencilView(depthStencilViews[DEPTH_STENCIL_VIEW::DEFAULT], D3D11_CLEAR_DEPTH, 1.0f, 0);

		// set constant buffers
		deviceContext_p->VSSetConstantBuffers(0, 1, &constantBuffers_vs_D3D[CONSTANT_BUFFER_VS::DEFAULT]);
		deviceContext_p->PSSetConstantBuffers(1, 1, &constantBuffers_ps_D3D[CONSTANT_BUFFER_PS::DEFAULT]);

		// set vertex shader constant buffer values
		SetCurrentWorldMatrix(GetDefaultWorldMatrix());
		SetCurrentViewMatrix(GetDefaultViewMatrix());
		SetCurrentProjectionMatrix(GetDefaultProjectionMatrix());
		UpdateVSConstantBuffer();

		// set pixel shader constant buffer values
		constantBuffers_ps[CONSTANT_BUFFER_PS::DEFAULT].light_pos = light_pos;
		constantBuffers_ps[CONSTANT_BUFFER_PS::DEFAULT].light_power = light_power;
		constantBuffers_ps[CONSTANT_BUFFER_PS::DEFAULT].light_color = light_color;
		constantBuffers_ps[CONSTANT_BUFFER_PS::DEFAULT].surface_shininess = surface_shininess;
		UpdatePSConstantBuffer();

		// set shaders
		deviceContext_p->VSSetShader(shaders_vertex[SHADER_VERTEX::DEFAULT], 0, 0);
		deviceContext_p->PSSetShader(shaders_pixel[SHADER_PIXEL::DEFAULT], 0, 0);

		// set samplers
		deviceContext_p->PSSetSamplers(0, 1, &samplerStates[SAMPLER_STATE::LINEAR]);


		// draw objects in scene if not disabled
		if (!(_draw_mode & DRAW_MODE::NO_SCENE_OBJECTS))
			for (uint32_t i = 0; i < sceneObjects.size(); i++)
			{
				DXVSetObject(sceneObjects[i]);
				D3DDrawIndexed(sceneObjects[i]->mesh_p->indexCount);
			}


		// draw debug lines if flag is set
		if (_draw_mode & DRAW_MODE::DRAW_DEBUG)
		{
			deviceContext_p->IASetPrimitiveTopology(DXV_PRIMITIVE_TOPOLOGY_DEBUG);


			deviceContext_p->PSSetShader(shaders_pixel[SHADER_PIXEL::DEBUG], nullptr, 0);
			deviceContext_p->IASetVertexBuffers(0, 1, &vertexBuffers[VERTEX_BUFFER::DEBUG], strides, offsets);
			deviceContext_p->UpdateSubresource(vertexBuffers[VERTEX_BUFFER::DEBUG], 0, NULL, DebugRenderer::get_line_verts(), 0, 0);

			SetCurrentWorldMatrix(XMMatrixIdentity());
			UpdateVSConstantBuffer();

			deviceContext_p->Draw((uint32_t)DebugRenderer::get_line_vert_count(), 0);


			deviceContext_p->IASetPrimitiveTopology(DXV_PRIMITIVE_TOPOLOGY_DEFAULT);
		}


		if (!(_draw_mode & DRAW_MODE::NO_PRESENT))
			Present();
	}
	void Present(UINT _syncInterval, UINT _flags) { swapChain_p->Present(_syncInterval, _flags); }

	void D3DDraw(uint32_t _numVerts) { deviceContext_p->Draw(_numVerts, 0); }
	void D3DDrawIndexed(uint32_t _numInds) { deviceContext_p->DrawIndexed(_numInds, 0, 0); }
#pragma endregion

#pragma region Mesh Functions
	HRESULT DXVLoadMeshData(const char* _filepath, DXVMESHDATA** _meshdata_pp)
	{
		HRESULT hr;
		std::fstream fin;

		hr = _OpenFile(_filepath, &fin);
		if (FAILED(hr)) return hr;

		DXVMESHDATA* meshdata_p = new DXVMESHDATA;

		// load data from file
		fin.read((char*)&meshdata_p->vertexCount, sizeof(meshdata_p->vertexCount));
		meshdata_p->vertices = new DXVVERTEX[meshdata_p->vertexCount];
		fin.read((char*)meshdata_p->vertices, meshdata_p->vertexCount * sizeof(DXVVERTEX));
		fin.read((char*)&meshdata_p->indexCount, sizeof(meshdata_p->indexCount));
		meshdata_p->indices = new uint32_t[meshdata_p->indexCount];
		fin.read((char*)meshdata_p->indices, meshdata_p->indexCount * sizeof(uint32_t));

		// store loaded data
		*_meshdata_pp = meshdata_p;

		return hr;
	}
	HRESULT DXVCreateMesh(const DXVMESHDATA* _meshdata_p, DXVMESH** _mesh_pp)
	{
		HRESULT hr = E_INVALIDARG;

		if (_meshdata_p == nullptr)
			errormsg = "Uninitialized DXVMESHDATA* passed to DXVCreateMesh";
		else
		{
			DXVMESH* mesh_p = new DXVMESH;

			// copy data
			mesh_p->vertexCount = _meshdata_p->vertexCount;
			mesh_p->indexCount = _meshdata_p->indexCount;

			hr = D3DCreateVertexBuffer(mesh_p->vertexCount, _meshdata_p->vertices, &mesh_p->vertexBuffer_p);
			if (FAILED(hr)) return hr;

			hr = D3DCreateIndexBuffer(mesh_p->indexCount, _meshdata_p->indices, &mesh_p->indexBuffer_p);
			if (FAILED(hr)) return hr;


			*_mesh_pp = mesh_p;
		}

		return hr;
	}
	HRESULT DXVLoadAndCreateMesh(const char* _filepath, DXVMESHDATA** _meshdata_pp, DXVMESH** _mesh_pp)
	{
		HRESULT hr;

		hr = DXVLoadMeshData(_filepath, _meshdata_pp);
		if (FAILED(hr)) return hr;

		return DXVCreateMesh(*_meshdata_pp, _mesh_pp);
	}
#pragma endregion

#pragma region Material Functions
	HRESULT DXVLoadMaterialData(const char* _filepath, DXVMATERIALDATA** _matdata_pp)
	{
		HRESULT hr;
		std::fstream fin;

		hr = _OpenFile(_filepath, &fin);
		if (FAILED(hr)) return hr;

		struct SIMPLEMATERIAL
		{
			struct COMPONENT
			{
				float value[3] = { 0.0f, 0.0f, 0.0f };
				float factor = 0.0f;
				int64_t input = -1;
			};

			enum ComponentType_e
			{
				Diffuse = 0
				, Emissive
				, Specular
				, NormalMap
				, Count
			};

			COMPONENT operator[](int i) { return components[i]; }
			const COMPONENT operator[](int i) const { return components[i]; }

			COMPONENT components[ComponentType_e::Count];
		};

		uint32_t numMats = 0;
		uint32_t numPaths = 0;
		filepath_t* paths_p = nullptr;
		SIMPLEMATERIAL* simpledata_p = nullptr;
		DXVMATERIALDATA* matdata_p = nullptr;

		// load data from file
		fin.read((char*)&numMats, sizeof(numMats));
		simpledata_p = new SIMPLEMATERIAL[numMats];
		matdata_p = new DXVMATERIALDATA[numMats];
		fin.read((char*)&simpledata_p[0], numMats * sizeof(SIMPLEMATERIAL));
		fin.read((char*)&numPaths, sizeof(numPaths));
		paths_p = new filepath_t[numPaths];
		fin.read((char*)&paths_p[0], numPaths * sizeof(filepath_t));

		// convert data to DXVMATERIALDATA format
		for (uint32_t i = 0; i < numMats; i++)
		{
			for (uint32_t c = 0; c < SIMPLEMATERIAL::ComponentType_e::Count; c++)
			{
				matdata_p[i].components[c].value.x = simpledata_p[i][c].value[0];
				matdata_p[i].components[c].value.y = simpledata_p[i][c].value[1];
				matdata_p[i].components[c].value.z = simpledata_p[i][c].value[2];

				matdata_p[i].components[c].factor = simpledata_p[i][c].factor;

				memcpy(matdata_p[i].components[c].filepath, (const void*)&paths_p[simpledata_p[i].components[c].input], 260);
			}
		}

		delete[] simpledata_p;
		delete[] paths_p;

		// store loaded data
		*_matdata_pp = matdata_p;

		return hr;
	}
	HRESULT DXVCreateMaterial(const DXVMATERIALDATA* _matdata_p, DXVMATERIAL** _material_pp)
	{
		HRESULT hr;

		if (_matdata_p == nullptr)
			errormsg = "Uninitialized DXVMATERIALDATA* passed to DXVCreateMaterial";
		else
		{
			DXVMATERIAL* material_p = new DXVMATERIAL;

			// copy data
			for (uint32_t c = 0; c < DXVMATERIAL::ComponentType_e::Count; c++)
			{
				material_p->components[c].value = _matdata_p->components[c].value;
				material_p->components[c].factor = _matdata_p->components[c].factor;

				const wchar_t* prefix = L"assets/";
				wchar_t partialpath[260];
				MultiByteToWideChar(CP_ACP, 0, _matdata_p->components[c].filepath, -1, (LPWSTR)partialpath, 260);

				wchar_t filepath[267];
				memcpy(&filepath[0], prefix, 7 * sizeof(wchar_t));
				memcpy(&filepath[7], partialpath, 260 * sizeof(wchar_t));

				hr = CreateWICTextureFromFile(device_p, filepath, &material_p->components[c].texture_p, &material_p->components[c].textureView_p);
				if (FAILED(hr))
				{
					errormsg = "Failed to create WIC texture from file";
					return hr;
				}
			}


			*_material_pp = material_p;
		}

		return hr;
		//return S_OK;
	}
	HRESULT DXVLoadAndCreateMaterial(const char* _filepath, DXVMATERIALDATA** _matdata_pp, DXVMATERIAL** _material_pp)
	{
		HRESULT hr;

		hr = DXVLoadMaterialData(_filepath, _matdata_pp);
		if (FAILED(hr)) return hr;

		return DXVCreateMaterial(*_matdata_pp, _material_pp);
	}
#pragma endregion

#pragma region Animation Functions
	HRESULT DXVLoadAnimationData(const char* _filepath, DXVANIMATIONDATA** _animdata_pp)
	{
		HRESULT hr;
		std::fstream fin;

		hr = _OpenFile(_filepath, &fin);
		if (FAILED(hr)) return hr;

		DXVANIMATIONDATA* animdata_p = new DXVANIMATIONDATA;

		// load data from file
		uint32_t numJoints = 0;
		uint32_t numFrames = 0;

		// read joint count
		fin.read((char*)&numJoints, sizeof(numJoints));
		animdata_p->bind_pose.joint_count = numJoints;

		// read bind pose joints
		animdata_p->bind_pose.joints = new DXVANIMATIONDATA::BINDPOSE::JOINT[numJoints];
		fin.read((char*)&animdata_p->bind_pose.joints[0], numJoints * sizeof(DXVANIMATIONDATA::BINDPOSE::JOINT));

		// read animation duration
		fin.read((char*)&animdata_p->duration, sizeof(animdata_p->duration));

		// read animation frame byte length
		fin.read((char*)&animdata_p->frame_byte_length, sizeof(animdata_p->frame_byte_length));

		// read frame count
		fin.read((char*)&numFrames, sizeof(numFrames));
		animdata_p->frame_count = numFrames;

		// read keyframe data
		animdata_p->frames = new DXVANIMATIONDATA::FRAME[numFrames];
		for (uint32_t i = 0; i < numFrames; i++)
		{
			animdata_p->frames[i].transforms = new float4x4[numJoints];
			fin.read((char*)&animdata_p->frames[i].time, sizeof(DXVANIMATIONDATA::FRAME::time));
			fin.read((char*)&animdata_p->frames[i].transforms[0], animdata_p->frame_byte_length - sizeof(DXVANIMATIONDATA::FRAME::time));
		}


		// store loaded data
		*_animdata_pp = animdata_p;

		return hr;
	}
	HRESULT DXVCreateAnimation(DXVANIMATIONDATA* _animdata_p, DXVANIMATION** _animation_pp)
	{
		HRESULT hr = E_INVALIDARG;

		if (_animdata_p == nullptr)
			errormsg = "Uninitialized DXVANIMATIONDATA* passed to DXVCreateAnimation";
		else
		{
			DXVANIMATION* animation_p = new DXVANIMATION;

			// copy joint count
			uint32_t numJoints = _animdata_p->bind_pose.joint_count;
			animation_p->bind_pose.joint_count = numJoints;

			// copy bind pose joints
			for (uint32_t i = 0; i < numJoints; i++)
			{
				DXVANIMATION::BINDPOSE::JOINT joint;
				joint.global_transform = Float4x4ToXMMatrix(_animdata_p->bind_pose.joints[i].global_transform);
				joint.parent_index = _animdata_p->bind_pose.joints[i].parent_index;
				animation_p->bind_pose.joints.push_back(joint);
			}

			// copy duration
			animation_p->duration = _animdata_p->duration;

			// copy frame count
			uint32_t numFrames = _animdata_p->frame_count;
			animation_p->frame_count = numFrames;

			// copy frame data
			for (uint32_t i = 0; i < numFrames; i++)
			{
				DXVANIMATION::FRAME frame;

				// copy time
				frame.time = _animdata_p->frames[i].time;

				// copy transforms
				for (uint32_t t = 0; t < numJoints; t++)
					frame.transforms.push_back(Float4x4ToDXVTransform(_animdata_p->frames[i].transforms[t]));

				animation_p->frames.push_back(frame);
			}

			hr = S_OK;


			*_animation_pp = animation_p;
		}

		return hr;
	}
	HRESULT DXVLoadAndCreateAnimation(const char* _filepath, DXVANIMATIONDATA** _animdata_pp, DXVANIMATION** _animation_pp)
	{
		HRESULT hr;

		hr = DXVLoadAnimationData(_filepath, _animdata_pp);
		if (FAILED(hr)) return hr;

		return DXVCreateAnimation(*_animdata_pp, _animation_pp);
	}

	void DXVInterpolateAnimationFrames(DXVANIMATION::FRAME& _frame, DXVANIMATION::FRAME _a, DXVANIMATION::FRAME _b, float _r)
	{
		_frame.time = lerp(_a.time, _b.time, _r);

		_frame.transforms.clear();

		for (uint32_t i = 0; i < _a.transforms.size(); i++)
		{
			DXVTRANSFORM transform = {};
			transform.translation = lerp(_a.transforms[i].translation, _b.transforms[i].translation, _r);
			transform.rotation = XMQuaternionSlerp(_a.transforms[i].rotation, _b.transforms[i].rotation, _r);
			_frame.transforms.push_back(transform);
		}
	}
#pragma endregion

#pragma region Object Functions
	HRESULT DXVLoadAndCreateObject(
		const char* _mesh_filepath, const char* _mat_filepath, const char* _anim_filepath,
		DXVMESHDATA** _meshdata_pp, DXVMESH** _mesh_pp,
		DXVMATERIALDATA** _matdata_pp, DXVMATERIAL** _material_pp,
		DXVANIMATIONDATA** _animdata_pp, DXVANIMATION** _animation_pp,
		DXVOBJECT* _object_p)
	{
		HRESULT hr;

		hr = DXVLoadAndCreateMesh(_mesh_filepath, _meshdata_pp, _mesh_pp);
		if (FAILED(hr)) return hr;

		hr = DXVLoadAndCreateMaterial(_mat_filepath, _matdata_pp, _material_pp);
		if (FAILED(hr)) return hr;

		hr = DXVLoadAndCreateAnimation(_anim_filepath, _animdata_pp, _animation_pp);
		if (FAILED(hr)) return hr;

		_object_p->mesh_p = *_mesh_pp;
		_object_p->material_p = *_material_pp;
		_object_p->animation_p = *_animation_pp;

		return hr;
	}

#pragma endregion

#pragma region Scene Functions
	void AddObjectToScene(DXVOBJECT* _obj_p) { if (_obj_p != nullptr) sceneObjects.push_back(_obj_p); }
	DXVOBJECT* GetObjectFromScene(uint16_t _i) { return sceneObjects[_i]; }
	void RemoveObjectFromScene(DXVOBJECT* _obj_p)
	{
		for (uint32_t i = 0; i < sceneObjects.size(); i++)
			if (sceneObjects[i] == _obj_p)
			{
				sceneObjects.erase(sceneObjects.begin() + i);
				break;
			}
	}
#pragma endregion

#pragma region Conversion Functions
	DXVTRANSFORM Float4x4ToDXVTransform(float4x4 _m)
	{
		DXVTRANSFORM t = {};
		t.translation = { _m[3].x, _m[3].y, _m[3].z, _m[3].w };
		XMMATRIX rotation =
		{
			_m[0].x, _m[0].y, _m[0].z, _m[0].w,
			_m[1].x, _m[1].y, _m[1].z, _m[1].w,
			_m[2].x, _m[2].y, _m[2].z, _m[2].w,
			0, 0, 0, 1
		};
		t.rotation = XMQuaternionRotationMatrix(rotation);
		return t;
	}
	DXVTRANSFORM XMMatrixToDXVTransform(XMMATRIX _m)
	{
		DXVTRANSFORM t = {};
		t.translation = _m.r[3];
		_m.r[3] = { 0, 0, 0, 1 };
		t.rotation = XMQuaternionRotationMatrix(_m);
		return t;
	}

	XMMATRIX DXVTransformToXMMatrix(DXVTRANSFORM _t)
	{
		XMMATRIX m = XMMatrixRotationQuaternion(_t.rotation);
		m.r[3] = _t.translation;
		return m;
	}
	XMMATRIX inline Float4x4ToXMMatrix(float4x4 _m)
	{
		return
		{
			_m[0].x, _m[0].y, _m[0].z, _m[0].w,
			_m[1].x, _m[1].y, _m[1].z, _m[1].w,
			_m[2].x, _m[2].y, _m[2].z, _m[2].w,
			_m[3].x, _m[3].y, _m[3].z, _m[3].w
		};
	}

	XMFLOAT3 inline XMVectorToXMFloat3(XMVECTOR _v) { return { XMVectorGetX(_v), XMVectorGetY(_v), XMVectorGetZ(_v) }; }
#pragma endregion

#pragma region Math Functions
	double inline lerp(double _a, double _b, double _r) { return (_b - _a) * _r + _a; }
	XMVECTOR lerp(XMVECTOR _a, XMVECTOR _b, double _r)
	{
		return {
			float(lerp(XMVectorGetX(_a), XMVectorGetX(_b), _r)),
			float(lerp(XMVectorGetY(_a), XMVectorGetY(_b), _r)),
			float(lerp(XMVectorGetZ(_a), XMVectorGetZ(_b), _r)),
			float(lerp(XMVectorGetW(_a), XMVectorGetW(_b), _r))
		};
	}
#pragma endregion

#pragma region D3D Helper Functions
	HRESULT D3DCreateDepthStencilView(uint32_t _w, uint32_t _h, ID3D11Texture2D** _depthStencil_pp, ID3D11DepthStencilView** _depthStencilView_pp)
	{
		HRESULT hr;

		D3D11_TEXTURE2D_DESC dsDesc = {};
		dsDesc.Width = _w;
		dsDesc.Height = _h;
		dsDesc.MipLevels = 1;
		dsDesc.ArraySize = 1;
		dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsDesc.SampleDesc.Count = 1;
		dsDesc.SampleDesc.Quality = 0;
		dsDesc.Usage = D3D11_USAGE_DEFAULT;
		dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		dsDesc.CPUAccessFlags = 0;
		dsDesc.MiscFlags = 0;
		hr = device_p->CreateTexture2D(&dsDesc, nullptr, _depthStencil_pp);
		if (FAILED(hr)) return hr;

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = dsDesc.Format;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;
		return device_p->CreateDepthStencilView(*_depthStencil_pp, &dsvDesc, _depthStencilView_pp);
	}
	HRESULT D3DCreateSamplerState(
		ID3D11SamplerState** _samplerState_pp, D3D11_FILTER _filter,
		D3D11_TEXTURE_ADDRESS_MODE _addressU, D3D11_TEXTURE_ADDRESS_MODE _addressV,
		D3D11_COMPARISON_FUNC _comp, float _minLod, float _maxLod)
	{
		D3D11_SAMPLER_DESC desc = {};
		desc.Filter = _filter;
		desc.AddressU = _addressU;
		desc.AddressV = _addressV;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.ComparisonFunc = _comp;
		desc.MinLOD = _minLod;
		desc.MaxLOD = _maxLod;
		return device_p->CreateSamplerState(&desc, _samplerState_pp);
	}
	HRESULT D3DCreateConstantBuffer(uint32_t _bytewidth, ID3D11Buffer** _cbuffer_pp)
	{
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = _bytewidth;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = 0;
		return device_p->CreateBuffer(&desc, nullptr, _cbuffer_pp);
	}
	HRESULT D3DCreateVertexBuffer(uint32_t _vertCount, DXVVERTEX* _verts, ID3D11Buffer** _vbuffer_pp)
	{
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = sizeof(DXVVERTEX) * _vertCount;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA subData = {};
		subData.pSysMem = _verts;
		return device_p->CreateBuffer(&desc, &subData, _vbuffer_pp);
	}
	HRESULT D3DCreateIndexBuffer(uint32_t _indCount, uint32_t* _inds, ID3D11Buffer** _ibuffer_pp)
	{
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = sizeof(uint32_t) * _indCount;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		desc.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA subData = {};
		subData.pSysMem = _inds;
		return device_p->CreateBuffer(&desc, &subData, _ibuffer_pp);
	}
	void D3DConfigureViewport(D3D11_VIEWPORT* _viewport_p, float _w, float _h, float _topLeftX, float _topLeftY, float _minDepth, float _maxDepth)
	{
		_viewport_p->Width = _w;
		_viewport_p->Height = _h;
		_viewport_p->TopLeftX = _topLeftX;
		_viewport_p->TopLeftY = _topLeftY;
		_viewport_p->MinDepth = _minDepth;
		_viewport_p->MaxDepth = _maxDepth;
	}

	void inline UpdateVSConstantBuffer()
	{
		deviceContext_p->UpdateSubresource(constantBuffers_vs_D3D[CONSTANT_BUFFER_VS::DEFAULT], 0, nullptr, &constantBuffers_vs[CONSTANT_BUFFER_VS::DEFAULT], 0, 0);
	}
	void inline UpdatePSConstantBuffer()
	{
		deviceContext_p->UpdateSubresource(constantBuffers_ps_D3D[CONSTANT_BUFFER_PS::DEFAULT], 0, nullptr, &constantBuffers_ps[CONSTANT_BUFFER_PS::DEFAULT], 0, 0);
	}
#pragma endregion

#pragma region Debug Functions
	void debug_AddMatrixToDebugRenderer(XMMATRIX _m, float _scale, bool _showNegativeAxes)
	{
		XMVECTOR matPos_v = _m.r[3];
		XMFLOAT3 matPos_f = XMVectorToXMFloat3(matPos_v);

		DebugRenderer::add_line(matPos_f, XMVectorToXMFloat3(matPos_v + _m.r[0] * _scale), RED_RGBA_FLOAT);
		DebugRenderer::add_line(matPos_f, XMVectorToXMFloat3(matPos_v + _m.r[1] * _scale), GREEN_RGBA_FLOAT);
		DebugRenderer::add_line(matPos_f, XMVectorToXMFloat3(matPos_v + _m.r[2] * _scale), BLUE_RGBA_FLOAT);

		if (_showNegativeAxes)
		{
			DebugRenderer::add_line(matPos_f, XMVectorToXMFloat3(matPos_v - _m.r[0] * _scale), CYAN_RGBA_FLOAT);
			DebugRenderer::add_line(matPos_f, XMVectorToXMFloat3(matPos_v - _m.r[1] * _scale), MAGENTA_RGBA_FLOAT);
			DebugRenderer::add_line(matPos_f, XMVectorToXMFloat3(matPos_v - _m.r[2] * _scale), YELLOW_RGBA_FLOAT);
		}
	}
	void debug_AddBoneToDebugRenderer(XMMATRIX _joint, XMMATRIX _parentJoint, bool _showJoint, XMFLOAT4 _color)
	{
		DebugRenderer::add_line(XMVectorToXMFloat3(_joint.r[3]), XMVectorToXMFloat3(_parentJoint.r[3]), _color);

		if (_showJoint)
			debug_AddMatrixToDebugRenderer(_joint, 0.25f);
	}
	void debug_AddSkeletonToDebugRenderer(DXVANIMATION::BINDPOSE* _bindpose_p, DXVANIMATION::FRAME* _frame_p, XMMATRIX _offset)
	{
		for (uint32_t i = 0; i < _bindpose_p->joint_count; i++)
		{
			int parentIndex = _bindpose_p->joints[i].parent_index;

			if (parentIndex >= 0)
			{
				XMMATRIX joint = DXVTransformToXMMatrix(_frame_p->transforms[i]) * _offset;
				XMMATRIX parentJoint = DXVTransformToXMMatrix(_frame_p->transforms[parentIndex]) * _offset;

				DirectXViewer::debug_AddBoneToDebugRenderer(joint, parentJoint, true);
			}
		}
	}
	void debug_AddSkeletonToDebugRenderer(DXVANIMATION::BINDPOSE* _bindpose_p, XMMATRIX _offset)
	{
		DXVANIMATION::FRAME frame = {};

		frame.time = -1.0f;

		for (uint32_t i = 0; i < _bindpose_p->joint_count; i++)
			frame.transforms.push_back(XMMatrixToDXVTransform(_bindpose_p->joints[i].global_transform));

		debug_AddSkeletonToDebugRenderer(_bindpose_p, &frame, _offset);
	}
#pragma endregion


	namespace DebugRenderer
	{
#pragma region Variables
		constexpr size_t capacity = 4096;

		size_t count = 0;
		std::array<DXVVERTEX, capacity> line_verts;
#pragma endregion

#pragma region Functions
		void add_line(XMFLOAT3 _point_a, XMFLOAT3 _point_b, XMFLOAT4 _color_a, XMFLOAT4 _color_b)
		{
			line_verts[count++] = { _point_a, {}, _color_a, {} };
			line_verts[count++] = { _point_b, {}, _color_b, {} };
		}
		void clear_lines() { count = 0; }
		const DXVVERTEX* get_line_verts() { return &line_verts[0]; }
		size_t get_line_vert_count() { return count; }
		size_t get_line_vert_capacity() { return capacity; }
#pragma endregion

	}

}
