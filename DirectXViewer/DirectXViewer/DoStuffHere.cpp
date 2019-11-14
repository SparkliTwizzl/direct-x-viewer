#include "DXVInterface.h"


namespace DXVInterface
{
#pragma region Variables
	ID3D11Device*						device_p = nullptr;
	ID3D11DeviceContext*				deviceContext_p = nullptr;
	IDXGISwapChain*						swapChain_p = nullptr;

	// Needed for manual raw D3D drawing
	uint32_t							strides[] = { sizeof(DirectXViewer::DXVVERTEX) };
	uint32_t							offsets[] = { 0 };

	// TODO: Add variables here
	XMFLOAT4							clearColor = GREY_DARK_RGBA_FLOAT32;

	const char*							testmesh_filename = "assets/Run.mesh";
	DirectXViewer::DXVMESHDATA*			testmeshdata_p = nullptr;
	DirectXViewer::DXVMESH*				testmesh_p = nullptr;

	const char*							testmat_filename = "assets/Run.mat";
	DirectXViewer::DXVMATERIALDATA*		testmatdata_p = nullptr;
	DirectXViewer::DXVMATERIAL*			testmat_p = nullptr;

	const char*							testanim_filename = "assets/Run.anim";
	/*
	28 joints
	0.708333 sec long
	1800 B frames
	20 frames
	37920 B file
	*/
	DirectXViewer::DXVANIMATION*		testanim_p = nullptr;

	DirectXViewer::DXVOBJECTDATA		testobjdata = {
		testmesh_filename,
		testmat_filename,
		testanim_filename,
		&testmeshdata_p,
		&testmesh_p,
		&testmatdata_p,
		&testmat_p,
		&testanim_p
	};
	DirectXViewer::DXVOBJECT			testobj;
#pragma endregion


#pragma region Private Helper Functions
	HRESULT ManualInit()
	{
		HRESULT hr;
		
		DirectXViewer::D3DSetClearColor(clearColor);

		XMVECTOR eye = { 0, 7, 7 };
		XMVECTOR at = { 0, 3, 0 };
		XMVECTOR up = { 0, 1, 0 };
		DirectXViewer::SetDefaultViewMatrix(XMMatrixLookAtLH(eye, at, up));

		hr = DirectXViewer::DXVLoadAndCreateObject(testobjdata, &testobj);
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
		delete testanim_p;
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

		// TODO: Add draw code here
		ManualDraw();

		uint32_t draw_mode = DirectXViewer::DRAW_MODE::DRAW_DEBUG;
		DirectXViewer::Draw(draw_mode); // If you are drawing manually after this function, pass DRAW_MODE::NO_PRESENT to Draw() and uncomment Present()

		//DirectXViewer::Present();
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
