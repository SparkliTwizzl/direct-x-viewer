#include <vector>

#include "DirectXViewer.h"

#include "vertexshader_default.csh"
#include "pixelshader_default.csh"


namespace DirectXViewer
{
	// window
	HWND*						hWnd_p = nullptr;

	uint32_t					windowWidth = 0;
	uint32_t					windowHeight = 0;

	// D3D
	D3D_DRIVER_TYPE				driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL			featureLevel = D3D_FEATURE_LEVEL_11_0;

	ID3D11Device*				device_p = nullptr;
	ID3D11DeviceContext*		deviceContext_p = nullptr;
	IDXGISwapChain*				swapChain_p = nullptr;

	ID3D11RenderTargetView*		renderTargetView_p = nullptr;
	ID3D11Texture2D*			depthStencil_p = nullptr;
	ID3D11DepthStencilView*		depthStencilView_p = nullptr;

	ID3D11InputLayout*			vertexLayout_p = nullptr;

	ID3D11SamplerState*			samplerLinear_p = nullptr;

	ID3D11Buffer*				cbuffer_vs_p = nullptr;
	ID3D11Buffer*				cbuffer_ps_p = nullptr;

	ID3D11VertexShader*			shader_vertex_p = nullptr;
	ID3D11PixelShader*			shader_pixel_p = nullptr;

	D3D11_VIEWPORT				viewport;

	XMFLOAT4X4					world;
	XMFLOAT4X4					view;
	XMFLOAT4X4					projection;

	std::vector<DXVOBJECT*>		sceneObjects;


	const char* errormsg;


	// TEST
	DXVVERTEX testverts[] =
	{
		{ { -1, 0, 0 }, { 0, 0, -1 }, {}, {} },
		{ { 0, 1, 0 }, { 0, 0, -1 }, {}, {} },
		{ { 1, 0, 0 }, { 0, 0, -1 }, {}, {} }
	};
	uint32_t testinds[] =
	{
		0, 1, 2
	};

	DXVMESHDATA* testmeshdata_p = nullptr;
	DXVMESH* testmesh_p = nullptr;
	// TEST


	HRESULT Init(HWND* _hWnd_p)
	{
		HRESULT hr;

		hWnd_p = _hWnd_p;

		// get window dimensions
		RECT wRect;
		GetClientRect(*hWnd_p, &wRect);
		windowWidth = wRect.right - wRect.left;
		windowHeight = wRect.bottom - wRect.top;


		// create device and swap chain
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
		hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
			createDeviceFlags, &featureLevel, 1, D3D11_SDK_VERSION,
			&swapChainDesc, &swapChain_p, &device_p, 0, &deviceContext_p);
		if (FAILED(hr)) return hr;

		// get back buffer from swap chain and create render target view
		ID3D11Resource* backbuffer_p = nullptr;
		hr = swapChain_p->GetBuffer(0, __uuidof(backbuffer_p), (void**)&backbuffer_p);
		if (FAILED(hr)) return hr;

		hr = device_p->CreateRenderTargetView(backbuffer_p, nullptr, &renderTargetView_p);
		if (FAILED(hr)) return hr;

		backbuffer_p->Release();

		// create depth stencil
		hr = DxCreateDepthStencilView(windowWidth, windowHeight, &depthStencil_p, &depthStencilView_p);

		// create and set input layout
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		UINT numInputElements = ARRAYSIZE(inputElementDesc);
		hr = device_p->CreateInputLayout(inputElementDesc, numInputElements, vertexshader_default, sizeof(vertexshader_default), &vertexLayout_p);
		if (FAILED(hr)) return hr;

		deviceContext_p->IASetInputLayout(vertexLayout_p);

		// create linear sampler
		hr = DxCreateSamplerState(&samplerLinear_p, D3D11_FILTER_MIN_MAG_MIP_LINEAR);
		if (FAILED(hr)) return hr;

		// create D3D constant buffers
		hr = DxCreateConstantBuffer(sizeof(DXVCBUFFER_VS), &cbuffer_vs_p);
		if (FAILED(hr)) return hr;

		hr = DxCreateConstantBuffer(sizeof(DXVCBUFFER_PS), &cbuffer_ps_p);
		if (FAILED(hr)) return hr;

		// create D3D shaders
		hr = device_p->CreateVertexShader(vertexshader_default, sizeof(vertexshader_default), nullptr, &shader_vertex_p);
		if (FAILED(hr)) return hr;

		hr = device_p->CreatePixelShader(pixelshader_default, sizeof(pixelshader_default), nullptr, &shader_pixel_p);
		if (FAILED(hr)) return hr;

		// set viewport values
		DxSetupViewport(&viewport, (float)windowWidth, (float)windowHeight);

		// set topology type
		deviceContext_p->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


		// initialize matrix values
		SetWorldMatrix(XMMatrixIdentity());

		XMVECTOR eye = { 0, 0, -5 };
		XMVECTOR at = { 0, 0, 0 };
		XMVECTOR up = { 0, 1, 0 };
		SetViewMatrix(XMMatrixLookAtLH(eye, at, up));

		SetProjectionMatrix(XMMatrixPerspectiveFovLH(XM_PIDIV4, windowWidth / (float)windowHeight, 0.01f, 100.0f));


		// TEST
		testmeshdata_p = new DXVMESHDATA;
		testmesh_p = new DXVMESH;

		testmeshdata_p->vertexCount = ARRAYSIZE(testverts);
		testmeshdata_p->vertices = testverts;
		testmeshdata_p->indexCount = ARRAYSIZE(testinds);
		testmeshdata_p->indices = testinds;

		hr = DXVCreateMesh(testmeshdata_p, &testmesh_p);
		// TEST


		return hr;
	}
	void Update()
	{
	}
	void Draw()
	{
		float clearcolor[] = BLACK;

		uint32_t strides[] = { sizeof(DXVVERTEX) };
		uint32_t offsets[] = { 0 };

		DXVCBUFFER_VS cbuffer_vs = {};
		DXVCBUFFER_PS cbuffer_ps = {};

		deviceContext_p->RSSetViewports(1, &viewport);
		deviceContext_p->OMSetRenderTargets(1, &renderTargetView_p, depthStencilView_p);
		deviceContext_p->VSSetConstantBuffers(0, 1, &cbuffer_vs_p);
		deviceContext_p->VSSetConstantBuffers(1, 1, &cbuffer_ps_p);
		deviceContext_p->ClearRenderTargetView(renderTargetView_p, clearcolor);
		deviceContext_p->ClearDepthStencilView(depthStencilView_p, D3D11_CLEAR_DEPTH, 1.0f, 0);

		cbuffer_vs.world = XMLoadFloat4x4(&world);
		cbuffer_vs.view = XMLoadFloat4x4(&view);
		cbuffer_vs.projection = XMLoadFloat4x4(&projection);

		deviceContext_p->UpdateSubresource(cbuffer_vs_p, 0, nullptr, &cbuffer_vs, 0, 0);
		deviceContext_p->UpdateSubresource(cbuffer_ps_p, 0, nullptr, &cbuffer_ps, 0, 0);

		deviceContext_p->VSSetShader(shader_vertex_p, 0, 0);
		deviceContext_p->PSSetShader(shader_pixel_p, 0, 0);

		deviceContext_p->PSSetSamplers(0, 1, &samplerLinear_p);


		// TEST
		deviceContext_p->IASetVertexBuffers(0, 1, &testmesh_p->vertexBuffer_p, strides, offsets);
		deviceContext_p->IASetIndexBuffer(testmesh_p->indexBuffer_p, DXGI_FORMAT_R32_UINT, 0);
		deviceContext_p->DrawIndexed(testmesh_p->indexCount, 0, 0);
		// TEST


		swapChain_p->Present(1, 0);
	}
	void Cleanup()
	{
#define RELEASE(p) if (p) p->Release()


		// TEST
		RELEASE(testmesh_p->indexBuffer_p);
		RELEASE(testmesh_p->vertexBuffer_p);
		// TEST


		RELEASE(shader_pixel_p);
		RELEASE(shader_vertex_p);

		RELEASE(cbuffer_ps_p);
		RELEASE(cbuffer_vs_p);

		RELEASE(samplerLinear_p);

		RELEASE(vertexLayout_p);

		RELEASE(depthStencilView_p);
		RELEASE(depthStencil_p);
		RELEASE(renderTargetView_p);

		RELEASE(swapChain_p);
		RELEASE(deviceContext_p);
		RELEASE(device_p);

#undef RELEASE(p)
	}


	const char* GetLastError() { return errormsg; }


	XMMATRIX GetWorldMatrix() { return XMLoadFloat4x4(&world); }
	XMMATRIX GetViewMatrix() { return XMLoadFloat4x4(&view); }
	XMMATRIX GetProjectionMatrix() { return XMLoadFloat4x4(&projection); }

	void SetWorldMatrix(XMMATRIX _m) { XMStoreFloat4x4(&world, _m); }
	void SetViewMatrix(XMMATRIX _m) { XMStoreFloat4x4(&view, _m); }
	void SetProjectionMatrix(XMMATRIX _m) { XMStoreFloat4x4(&projection, _m); }


	HRESULT DXVCreateMesh(DXVMESHDATA* _meshdata_p, DXVMESH** _mesh_pp)
	{
		HRESULT hr;

		DXVMESH* mesh_p = new DXVMESH;

		mesh_p->vertexCount = _meshdata_p->vertexCount;
		mesh_p->indexCount = _meshdata_p->indexCount;

		hr = DxCreateVertexBuffer(mesh_p->vertexCount, _meshdata_p->vertices, &mesh_p->vertexBuffer_p);
		if (FAILED(hr)) return hr;

		hr = DxCreateIndexBuffer(mesh_p->indexCount, _meshdata_p->indices, &mesh_p->indexBuffer_p);
		if (FAILED(hr)) return hr;

		*_mesh_pp = mesh_p;

		return S_OK;
	}


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


	HRESULT DxCreateDepthStencilView(uint32_t _w, uint32_t _h, ID3D11Texture2D** _depthStencil_pp, ID3D11DepthStencilView** _depthStencilView_pp)
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
	HRESULT DxCreateSamplerState(
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
		return device_p->CreateSamplerState(&desc, &samplerLinear_p);
	}
	HRESULT DxCreateConstantBuffer(uint32_t _bytewidth, ID3D11Buffer** _cbuffer_pp)
	{
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = _bytewidth;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = 0;
		return device_p->CreateBuffer(&desc, nullptr, _cbuffer_pp);
	}
	HRESULT DxCreateVertexBuffer(uint32_t _vertCount, DXVVERTEX* _verts, ID3D11Buffer** _vbuffer_pp)
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
	HRESULT DxCreateIndexBuffer(uint32_t _indCount, uint32_t* _inds, ID3D11Buffer** _ibuffer_pp)
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
	void DxSetupViewport(D3D11_VIEWPORT* _viewport_p, float _w, float _h, float _topLeftX, float _topLeftY, float _minDepth, float _maxDepth)
	{
		_viewport_p->Width = _w;
		_viewport_p->Height = _h;
		_viewport_p->TopLeftX = _topLeftX;
		_viewport_p->TopLeftY = _topLeftY;
		_viewport_p->MinDepth = _minDepth;
		_viewport_p->MaxDepth = _maxDepth;
	}

}
