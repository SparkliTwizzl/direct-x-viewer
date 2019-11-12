#include <bitset>
#include <fstream>
#include <vector>

#include"WICTextureLoader.h"

#include "vertexshader_default.csh"
#include "pixelshader_default.csh"

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

	ID3D11RenderTargetView*			renderTargetView_p = nullptr;
	ID3D11Texture2D*				depthStencil_p = nullptr;
	ID3D11DepthStencilView*			depthStencilView_p = nullptr;

	ID3D11InputLayout*				vertexLayout_p = nullptr;

	ID3D11SamplerState*				samplerLinear_p = nullptr;

	DXVCBUFFER_VS					cbuffer_vs;
	DXVCBUFFER_PS					cbuffer_ps;

	ID3D11Buffer*					cbuffer_d3d_vs_p = nullptr;
	ID3D11Buffer*					cbuffer_d3d_ps_p = nullptr;

	ID3D11VertexShader*				shader_vertex_p = nullptr;
	ID3D11PixelShader*				shader_pixel_p = nullptr;

	D3D11_VIEWPORT					viewport;

	uint32_t						strides[] = { sizeof(DXVVERTEX) };
	uint32_t						offsets[] = { 0 };
#pragma endregion

#pragma region DXV Variables
	const char*						errormsg;

	XMFLOAT4X4						world;
	XMFLOAT4X4						view;
	XMFLOAT4X4						projection;

	float							clearToColor[4] = BLACK_RGBA_FLOAT32;

	XMFLOAT3						light_pos = { 0.0f, 5.0f, 5.0f };
	XMFLOAT3						light_color = WHITE_RGB_FLOAT32;
	float							light_power = 10.0f;
	float							light_rotationSpeed = 2.0f;

	float							surface_shininess = 10.0f;

	std::vector<DXVOBJECT*>			sceneObjects;
#pragma endregion

#pragma region Input Variables
#define INPUT_X_NEG 'A'
#define INPUT_X_POS 'D'
#define INPUT_Y_NEG VK_SHIFT
#define INPUT_Y_POS VK_SPACE
#define INPUT_Z_NEG 'S'
#define INPUT_Z_POS 'W'

	enum Inputs_e
	{
		CamTranslateXNeg = 0
		, CamTranslateXPos
		, CamTranslateYNeg
		, CamTranslateYPos
		, CamTranslateZNeg
		, CamTranslateZPos
		, CamRotate
		, Count
	};

	std::bitset<Inputs_e::Count>	inputValues;

	int32_t							xMouse;
	int32_t							yMouse;
	int32_t							xMouse_prev;
	int32_t							yMouse_prev;

	const float						camTranslationSpeed = 4.0f;
	const float						camRotationSpeed = 0.1f;
#pragma endregion


#pragma region Private Helper Functions
	// setters

	// sets the inverse-transpose world matrix (used for normal vectors in shaders)
	void SetWorldITMatrix(XMMATRIX _m) { cbuffer_vs.worldIT = XMMatrixTranspose(XMMatrixInverse(nullptr, _m)); }


	// init

	HRESULT	InitD3DResources()
	{
		HRESULT hr;

		errormsg = "D3D initialization failed";


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
		hr = D3DCreateDepthStencilView(windowWidth, windowHeight, &depthStencil_p, &depthStencilView_p);


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
		hr = D3DCreateSamplerState(&samplerLinear_p, D3D11_FILTER_MIN_MAG_MIP_LINEAR);
		if (FAILED(hr)) return hr;


		// create D3D constant buffers
		hr = D3DCreateConstantBuffer(sizeof(DXVCBUFFER_VS), &cbuffer_d3d_vs_p);
		if (FAILED(hr)) return hr;

		hr = D3DCreateConstantBuffer(sizeof(DXVCBUFFER_PS), &cbuffer_d3d_ps_p);
		if (FAILED(hr)) return hr;


		// create D3D shaders
		hr = device_p->CreateVertexShader(vertexshader_default, sizeof(vertexshader_default), nullptr, &shader_vertex_p);
		if (FAILED(hr)) return hr;

		hr = device_p->CreatePixelShader(pixelshader_default, sizeof(pixelshader_default), nullptr, &shader_pixel_p);
		if (FAILED(hr)) return hr;


		// set viewport values
		D3DConfigureViewport(&viewport, (float)windowWidth, (float)windowHeight);


		// set topology type
		deviceContext_p->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


		return hr;
	}
	HRESULT OpenFile(const char* _filepath, std::fstream* _fin)
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


	// update

	void ReadInputs(const MSG* _msg)
	{
		// camera translation
		if (_msg->message == WM_KEYDOWN || _msg->message == WM_KEYUP)
		{
			bool value = _msg->message == WM_KEYDOWN;

			switch (_msg->wParam)
			{
			case INPUT_X_NEG:
				inputValues.set(Inputs_e::CamTranslateXNeg, value);
				break;
			case INPUT_X_POS:
				inputValues.set(Inputs_e::CamTranslateXPos, value);
				break;
			case INPUT_Y_NEG:
				inputValues.set(Inputs_e::CamTranslateYNeg, value);
				break;
			case INPUT_Y_POS:
				inputValues.set(Inputs_e::CamTranslateYPos, value);
				break;
			case INPUT_Z_NEG:
				inputValues.set(Inputs_e::CamTranslateZNeg, value);
				break;
			case INPUT_Z_POS:
				inputValues.set(Inputs_e::CamTranslateZPos, value);
				break;
			default:
				break;
			}
		}

		// camera rotation
		if (_msg->message == WM_RBUTTONDOWN)
		{
			inputValues.set(Inputs_e::CamRotate, true);
		}
		if (_msg->message == WM_RBUTTONUP)
		{
			inputValues.set(Inputs_e::CamRotate, false);
		}
		if (_msg->message == WM_MOUSEMOVE)
		{
			xMouse_prev = xMouse;
			yMouse_prev = yMouse;
			xMouse = LOWORD(_msg->lParam);
			yMouse = HIWORD(_msg->lParam);
		}
	}
	void UpdateCamera(float _dt)
	{
		XMMATRIX mView = XMMatrixInverse(nullptr, XMLoadFloat4x4(&view));

		float t_camTranslationSpeed = camTranslationSpeed * _dt
			, t_camRotationSpeed = camRotationSpeed * _dt;

		float dX = 0.0f
			, dY = 0.0f
			, dZ = 0.0f
			, drX = 0.0f
			, drY = 0.0f;


		// get translation amounts

		// X
		if (inputValues.test(Inputs_e::CamTranslateXNeg))
			dX -= t_camTranslationSpeed;
		if (inputValues.test(Inputs_e::CamTranslateXPos))
			dX += t_camTranslationSpeed;

		// Y
		if (inputValues.test(Inputs_e::CamTranslateYNeg))
			dY -= t_camTranslationSpeed;
		if (inputValues.test(Inputs_e::CamTranslateYPos))
			dY += t_camTranslationSpeed;

		// Z
		if (inputValues.test(Inputs_e::CamTranslateZNeg))
			dZ -= t_camTranslationSpeed;
		if (inputValues.test(Inputs_e::CamTranslateZPos))
			dZ += t_camTranslationSpeed;


		// apply translations
		mView = ((XMMatrixTranslation(dX, 0, 0) * mView) * XMMatrixTranslation(0, dY, 0)) * XMMatrixTranslationFromVector(XMVector3Cross(mView.r[0], { 0, 1, 0 }) * dZ);


		if (inputValues.test(Inputs_e::CamRotate))
		{
			// get rotation amounts
			float camRotY = (float)(xMouse - xMouse_prev) * t_camRotationSpeed
				, camRotX = (float)(yMouse - yMouse_prev) * t_camRotationSpeed;

			// get view matrix offset from origin
			XMVECTOR offset = { XMVectorGetX(mView.r[3]), XMVectorGetY(mView.r[3]), XMVectorGetZ(mView.r[3]) };

			// temporarily shift view matrix to origin and apply rotations
			mView = mView * XMMatrixTranslationFromVector(-offset);
			mView = (XMMatrixRotationX(camRotX) * mView) * XMMatrixRotationY(camRotY);
			mView = mView * XMMatrixTranslationFromVector(offset);
		}


		XMStoreFloat4x4(&view, XMMatrixInverse(nullptr, mView));
	}
#pragma endregion

#pragma region Getters
	const char* GetLastError() { return errormsg; }

	XMMATRIX GetDefaultWorldMatrix() { return XMLoadFloat4x4(&world); }
	XMMATRIX GetDefaultViewMatrix() { return XMLoadFloat4x4(&view); }
	XMMATRIX GetDefaultProjectionMatrix() { return XMLoadFloat4x4(&projection); }
	XMMATRIX GetCurrentWorldMatrix() { return cbuffer_vs.world; }
	XMMATRIX GetCurrentViewMatrix() { return cbuffer_vs.view; }
	XMMATRIX GetCurrentProjectionMatrix() { return cbuffer_vs.projection; }

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
		cbuffer_vs.world = _m;
		SetWorldITMatrix(_m);
	}
	void SetCurrentViewMatrix(XMMATRIX _m) { cbuffer_vs.view = _m; }
	void SetCurrentProjectionMatrix(XMMATRIX _m) { cbuffer_vs.projection = _m; }

	void D3DSetVertexBuffer(ID3D11Buffer** _vbuffer_pp) { deviceContext_p->IASetVertexBuffers(0, 1, _vbuffer_pp, strides, offsets); }
	void D3DSetIndexBuffer(ID3D11Buffer* _ibuffer_p) { deviceContext_p->IASetIndexBuffer(_ibuffer_p, IBUFFER_FORMAT, 0); }
	void D3DSetDiffuseMaterial(ID3D11ShaderResourceView* _material_p) { deviceContext_p->PSSetShaderResources(0, 1, &_material_p); }
	void D3DSetEmissiveMaterial(ID3D11ShaderResourceView* _material_p) { deviceContext_p->PSSetShaderResources(1, 1, &_material_p); }
	void D3DSetSpecularMaterial(ID3D11ShaderResourceView* _material_p) { deviceContext_p->PSSetShaderResources(2, 1, &_material_p); }
	void D3DSetNormalMapMaterial(ID3D11ShaderResourceView* _material_p) { deviceContext_p->PSSetShaderResources(3, 1, &_material_p); }
	void D3DSetClearToColor(float _color[4])
	{
		clearToColor[0] = _color[0];
		clearToColor[1] = _color[1];
		clearToColor[2] = _color[2];
		clearToColor[3] = _color[3];
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
		SetCurrentWorldMatrix(_object_p->modeling);
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
		hr = InitD3DResources();
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
	void Update(const MSG* _msg)
	{
		// get clock ticks since previous call
		static uint64_t startTime = 0;
		static uint64_t prevTime = 0;
		uint64_t curTime = GetTickCount64();
		if (startTime == 0)
			startTime = prevTime = curTime;
		float t = (curTime - startTime) / 1000.0f;
		float dt = (curTime - prevTime) / 1000.0f;
		prevTime = curTime;



		// orbit light around origin
		XMVECTOR t_light_pos = { light_pos.x, light_pos.y, light_pos.z };
		t_light_pos = (XMMatrixTranslationFromVector(t_light_pos) * XMMatrixRotationY(light_rotationSpeed * dt)).r[3];
		light_pos = { XMVectorGetX(t_light_pos), XMVectorGetY(t_light_pos), XMVectorGetZ(t_light_pos) };


		// read input and update camera
		ReadInputs(_msg);
		UpdateCamera(dt);
	}
	void Cleanup()
	{
		// release D3D resources
#define RELEASE(p) if (p) p->Release()

		RELEASE(shader_pixel_p);
		RELEASE(shader_vertex_p);

		RELEASE(cbuffer_d3d_ps_p);
		RELEASE(cbuffer_d3d_vs_p);

		RELEASE(samplerLinear_p);

		RELEASE(vertexLayout_p);

		RELEASE(depthStencilView_p);
		RELEASE(depthStencil_p);
		RELEASE(renderTargetView_p);

		RELEASE(swapChain_p);
		RELEASE(deviceContext_p);
		RELEASE(device_p);

#undef RELEASE

		sceneObjects.clear();

		// uninitialize WIC texture loader
		CoUninitialize();
	}
#pragma endregion

#pragma region Draw Functions
	void Draw(bool _present)
	{
		// clear constant buffer data
		cbuffer_vs = {};
		cbuffer_ps = {};

		// set viewport
		deviceContext_p->RSSetViewports(1, &viewport);

		// set and clear render target and depth stencil
		deviceContext_p->OMSetRenderTargets(1, &renderTargetView_p, depthStencilView_p);
		deviceContext_p->ClearRenderTargetView(renderTargetView_p, clearToColor);
		deviceContext_p->ClearDepthStencilView(depthStencilView_p, D3D11_CLEAR_DEPTH, 1.0f, 0);

		// set constant buffers
		deviceContext_p->VSSetConstantBuffers(0, 1, &cbuffer_d3d_vs_p);
		deviceContext_p->PSSetConstantBuffers(1, 1, &cbuffer_d3d_ps_p);

		// set vertex shader constant buffer values
		SetCurrentWorldMatrix(GetDefaultWorldMatrix());
		SetCurrentViewMatrix(GetDefaultViewMatrix());
		SetCurrentProjectionMatrix(GetDefaultProjectionMatrix());
		UpdateVSConstantBuffer();

		// set pixel shader constant buffer values
		cbuffer_ps.light_pos = light_pos;
		cbuffer_ps.light_power = light_power;
		cbuffer_ps.light_color = light_color;
		cbuffer_ps.surface_shininess = surface_shininess;
		UpdatePSConstantBuffer();

		//cbuffer_ps.light_pos = { 0, 10, 0 };
		//cbuffer_ps.light_power = 1.0f;
		//cbuffer_ps.light_color = { 1, 1, 1 };
		//cbuffer_ps.surface_shininess = 1.0f;
		//deviceContext_p->UpdateSubresource(cbuffer_d3d_ps_p, 0, nullptr, &cbuffer_ps, 0, 0);

		// set shaders
		deviceContext_p->VSSetShader(shader_vertex_p, 0, 0);
		deviceContext_p->PSSetShader(shader_pixel_p, 0, 0);

		// set samplers
		deviceContext_p->PSSetSamplers(0, 1, &samplerLinear_p);


		// draw objects in scene
		for (uint32_t i = 0; i < sceneObjects.size(); i++)
		{
			DXVSetObject(sceneObjects[i]);
			D3DDrawIndexed(sceneObjects[i]->mesh_p->indexCount);
		}


		if (_present)
			Present();
	}
	void Present(UINT _syncInterval, UINT _flags) { swapChain_p->Present(_syncInterval, _flags); }

	void D3DDraw(uint32_t _numVerts) { deviceContext_p->Draw(_numVerts, 0); }
	void D3DDrawIndexed(uint32_t _numInds) { deviceContext_p->DrawIndexed(_numInds, 0, 0); }
#pragma endregion

#pragma region Mesh/Material/Object Functions
	HRESULT DXVLoadMeshData(const char* _filepath, DXVMESHDATA** _meshdata_pp)
	{
		HRESULT hr;
		std::fstream fin;

		hr = OpenFile(_filepath, &fin);
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

		hr = DXVCreateMesh(*_meshdata_pp, _mesh_pp);
		return hr;
	}

	HRESULT DXVLoadMaterialData(const char* _filepath, DXVMATERIALDATA** _matdata_pp)
	{
		HRESULT hr;
		std::fstream fin;

		hr = OpenFile(_filepath, &fin);
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

		DXVMATERIAL* material_p = new DXVMATERIAL;

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
		// FOR EACH COMPONENT :

		/*
		HRESULT CreateWICTextureFromFile(
			_In_ ID3D11Device* d3dDevice,								// device_p
			_In_z_ const wchar_t* szFileName,							// material_p.components[c].path
			_Outptr_opt_ ID3D11Resource** texture,						// material_p.components[c].texture_p
			_Outptr_opt_ ID3D11ShaderResourceView** textureView,		// material_p.components[c].textureView_p
			_In_ size_t maxsize = 0)
		*/


		*_material_pp = material_p;

		return hr;
		//return S_OK;
	}
	HRESULT DXVLoadAndCreateMaterial(const char* _filepath, DXVMATERIALDATA** _matdata_pp, DXVMATERIAL** _material_pp)
	{
		HRESULT hr;

		hr = DXVLoadMaterialData(_filepath, _matdata_pp);
		if (FAILED(hr)) return hr;

		hr = DXVCreateMaterial(*_matdata_pp, _material_pp);
		return hr;
	}

	HRESULT DXVLoadAndCreateObject(
		const char* _mesh_filepath, const char* _mat_filepath,
		DXVMESHDATA** _meshdata_pp, DXVMESH** _mesh_pp,
		DXVMATERIALDATA** _matdata_pp, DXVMATERIAL** _material_pp,
		DXVOBJECT* _object_p)
	{
		HRESULT hr;

		hr = DXVLoadAndCreateMesh(_mesh_filepath, _meshdata_pp, _mesh_pp);
		if (FAILED(hr)) return hr;

		hr = DXVLoadAndCreateMaterial(_mat_filepath, _matdata_pp, _material_pp);
		if (FAILED(hr)) return hr;

		_object_p->mesh_p = *_mesh_pp;
		_object_p->material_p = *_material_pp;

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
		return device_p->CreateSamplerState(&desc, &samplerLinear_p);
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

	void UpdateVSConstantBuffer() { deviceContext_p->UpdateSubresource(cbuffer_d3d_vs_p, 0, nullptr, &cbuffer_vs, 0, 0); }
	void UpdatePSConstantBuffer() { deviceContext_p->UpdateSubresource(cbuffer_d3d_ps_p, 0, nullptr, &cbuffer_ps, 0, 0); }
#pragma endregion

}
