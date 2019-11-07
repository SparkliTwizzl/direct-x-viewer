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
	DirectXViewer::DXVOBJECT			testobj1;
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
		DirectXViewer::SetDefaultViewMatrix(XMMatrixLookAtLH(eye, at, up));


		hr = DirectXViewer::DXVLoadMeshData("assets/Idle.mesh", &testmeshdata_p);
		if (FAILED(hr)) return hr;

		hr = DirectXViewer::DXVCreateMesh(testmeshdata_p, &testmesh_p);
		if (FAILED(hr)) return hr;


		hr = DirectXViewer::DXVLoadMaterialData("assets/Idle.mat", &testmatdata_p);
		if (FAILED(hr)) return hr;

		hr = DirectXViewer::DXVCreateMaterial(&testmatdata_p[0], &testmat_p);
		if (FAILED(hr)) return hr;


		testobj.modeling = XMMatrixTranslation(-2.5f, 0, 0);
		testobj.mesh_p = testmesh_p;
		testobj.material_p = testmat_p;

		testobj1.modeling = XMMatrixTranslation(2.5f, 0, 0);
		testobj1.mesh_p = testmesh_p;
		testobj1.material_p = testmat_p;


		DirectXViewer::AddObjectToScene(&testobj);
		DirectXViewer::AddObjectToScene(&testobj1);


		return hr;
	}
	void Update(const MSG* _msg)
	{
		DirectXViewer::Update(_msg);

		// TODO: Add update code here


		// If you are drawing manually, pass false to Draw() and uncomment Present() call at end of function
		DirectXViewer::Draw();

		// Needed for manual raw D3D drawing
		uint32_t strides[] = { sizeof(DirectXViewer::DXVVERTEX) };
		uint32_t offsets[] = { 0 };

		// TODO: Add draw code here


		// Uncomment this if you are drawing anything manually
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
