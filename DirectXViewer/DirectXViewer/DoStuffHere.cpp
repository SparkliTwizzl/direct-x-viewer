#include "DXVInterface.h"


namespace DXVInterface
{
#pragma region Variables
	ID3D11Device*						device_p = nullptr;
	ID3D11DeviceContext*				deviceContext_p = nullptr;
	IDXGISwapChain*						swapChain_p = nullptr;

	// Needed for manual raw D3D drawing
	uint32_t strides[] = { sizeof(DirectXViewer::DXVVERTEX) };
	uint32_t offsets[] = { 0 };

	// TODO: Add variables here
	DirectXViewer::DXVMESHDATA*			testmeshdata_p = nullptr;
	DirectXViewer::DXVMESH*				testmesh_p = nullptr;

	DirectXViewer::DXVMATERIALDATA*		testmatdata_p = nullptr;
	DirectXViewer::DXVMATERIAL*			testmat_p = nullptr;

	DirectXViewer::DXVOBJECT			testobj;
#pragma endregion


#pragma region Private Helper Functions
	HRESULT ManualInit()
	{
		HRESULT hr;


		XMVECTOR eye = { 0, 3, 10 };
		XMVECTOR at = { 0, 3, 0 };
		XMVECTOR up = { 0, 1, 0 };
		DirectXViewer::SetDefaultViewMatrix(XMMatrixLookAtLH(eye, at, up));

		hr = DirectXViewer::DXVLoadAndCreateObject("assets/Idle.mesh", "assets/Idle.mat", &testmeshdata_p, &testmesh_p, &testmatdata_p, &testmat_p, &testobj);
		if (FAILED(hr)) return hr;

		DirectXViewer::AddObjectToScene(&testobj);


		return hr;
	}
	void ManualUpdate()
	{
	}
	void ManualDraw()
	{
	}
	void ManualCleanup()
	{
		delete testmat_p;
		delete testmatdata_p;
		delete testmesh_p;
		delete testmeshdata_p;
	}
#pragma endregion


#pragma region Basic Functions
	HRESULT Init(HWND* _hWnd_p)
	{
		HRESULT hr;

		// Automatic initialization
		hr = DirectXViewer::Init(_hWnd_p);
		if (FAILED(hr)) return hr;

		device_p = DirectXViewer::GetDevice();
		deviceContext_p = DirectXViewer::GetDeviceContext();
		swapChain_p = DirectXViewer::GetSwapChain();

		// TODO: Add init code here
		hr = ManualInit();
		if (FAILED(hr)) return hr;

		return hr;
	}
	void Update(const MSG* _msg)
	{
		DirectXViewer::Update(_msg);

		// TODO: Add update code here
		ManualUpdate();

		DirectXViewer::Draw(false); // If you are drawing manually, pass false to Draw() and uncomment Present()

		// TODO: Add draw code here
		ManualDraw();

		DirectXViewer::Present(); // Uncomment this if you are drawing anything manually
	}
	void Cleanup()
	{
		// TODO: Add cleanup code here
		ManualCleanup();

		// Automatic cleanup
		swapChain_p = nullptr;
		deviceContext_p = nullptr;
		device_p = nullptr;
		DirectXViewer::Cleanup();
	}
#pragma endregion

}
