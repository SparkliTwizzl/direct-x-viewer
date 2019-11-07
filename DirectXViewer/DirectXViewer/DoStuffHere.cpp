#include "DXVInterface.h"


namespace DXVInterface
{
#pragma region Variables
	ID3D11Device*						device_p = nullptr;
	ID3D11DeviceContext*				deviceContext_p = nullptr;
	IDXGISwapChain*						swapChain_p = nullptr;

	// TODO: Add variables here
	DirectXViewer::DXVMESHDATA*			testmeshdata_p = nullptr;
	DirectXViewer::DXVMESH*				testmesh_p = nullptr;

	DirectXViewer::DXVMATERIALDATA*		testmatdata_p = nullptr;
	DirectXViewer::DXVMATERIAL*			testmat_p = nullptr;

	DirectXViewer::DXVOBJECT			testobj;
#pragma endregion


#pragma region Basic Functions
	HRESULT Init(HWND* _hWnd_p)
	{
		HRESULT hr;

		hr = DirectXViewer::Init(_hWnd_p);
		if (FAILED(hr)) return hr;

		device_p = DirectXViewer::GetDevice();
		deviceContext_p = DirectXViewer::GetDeviceContext();
		swapChain_p = DirectXViewer::GetSwapChain();

		// TODO: Add init code here
		XMVECTOR eye = { 0, 3, 10 };
		XMVECTOR at = { 0, 3, 0 };
		XMVECTOR up = { 0, 1, 0 };
		DirectXViewer::SetViewMatrix(XMMatrixLookAtLH(eye, at, up));


		hr = DirectXViewer::DXVLoadMeshData("assets/Idle.mesh", &testmeshdata_p);
		if (FAILED(hr)) return hr;

		hr = DirectXViewer::DXVCreateMesh(testmeshdata_p, &testmesh_p);
		if (FAILED(hr)) return hr;


		hr = DirectXViewer::DXVLoadMaterialData("assets/Idle.mat", &testmatdata_p);
		if (FAILED(hr)) return hr;

		hr = DirectXViewer::DXVCreateMaterial(&testmatdata_p[0], &testmat_p);
		if (FAILED(hr)) return hr;


		testobj.mesh_p = testmesh_p;
		testobj.material_p = testmat_p;


		DirectXViewer::AddObjectToScene(&testobj);


		return hr;
	}
	void Update(const MSG* _msg)
	{
		DirectXViewer::Update(_msg);

		// TODO: Add update code here


		// If you are drawing manually, pass false to Draw() and call Present() after manual draws
		DirectXViewer::Draw();

		//uint32_t strides[] = { sizeof(DirectXViewer::DXVVERTEX) };
		//uint32_t offsets[] = { 0 };

		// TODO: Add draw code here
		//DirectXViewer::D3DSetVertexBuffer(&testobj.mesh_p->vertexBuffer_p);
		//DirectXViewer::D3DSetIndexBuffer(testobj.mesh_p->indexBuffer_p);

		//DirectXViewer::D3DSetDiffuseMaterial(testobj.material_p->components[DirectXViewer::DXVMATERIAL::ComponentType_e::Diffuse].textureView_p);
		//DirectXViewer::D3DSetEmissiveMaterial(testobj.material_p->components[DirectXViewer::DXVMATERIAL::ComponentType_e::Emissive].textureView_p);
		//DirectXViewer::D3DSetSpecularMaterial(testobj.material_p->components[DirectXViewer::DXVMATERIAL::ComponentType_e::Specular].textureView_p);

		//DirectXViewer::DXVSetObject(&testobj);

		//DirectXViewer::D3DDrawIndexed(testobj.mesh_p->indexCount);

		//DirectXViewer::Present();
	}
	void Cleanup()
	{
		// TODO: Add cleanup code here
		delete testmat_p;
		delete testmatdata_p;
		delete testmesh_p;
		delete testmeshdata_p;


		swapChain_p = nullptr;
		deviceContext_p = nullptr;
		device_p = nullptr;
		DirectXViewer::Cleanup();
	}
#pragma endregion

}
