#include "Interface.h"


namespace Interface
{
#pragma region Variables
	ID3D11Device*					device_p = nullptr;
	ID3D11DeviceContext*			deviceContext_p = nullptr;
	IDXGISwapChain*					swapChain_p = nullptr;

	DirectXViewer::DXVMESHDATA* testmeshdata_p = nullptr;
	DirectXViewer::DXVMESH* testmesh_p = nullptr;
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
		XMVECTOR eye = { 0, 3, -10 };
		XMVECTOR at = { 0, 3, 0 };
		XMVECTOR up = { 0, 1, 0 };
		DirectXViewer::SetViewMatrix(XMMatrixLookAtLH(eye, at, up));

		hr = DirectXViewer::DXVLoadMeshData("assets/Idle.mesh", &testmeshdata_p);
		if (FAILED(hr)) return hr;

		hr = DirectXViewer::DXVCreateMesh(testmeshdata_p, &testmesh_p);
		if (FAILED(hr)) return hr;


		return hr;
	}
	void Update()
	{
		DirectXViewer::Update();

		// TODO: Add update code here


		// If you are drawing manually, pass false to Draw() and call Present() after manual draws
		DirectXViewer::Draw(false);

		uint32_t strides[] = { sizeof(DirectXViewer::DXVVERTEX) };
		uint32_t offsets[] = { 0 };

		// TODO: Add draw code here
		deviceContext_p->IASetVertexBuffers(0, 1, &testmesh_p->vertexBuffer_p, strides, offsets);
		deviceContext_p->IASetIndexBuffer(testmesh_p->indexBuffer_p, IBUFFER_FORMAT, 0);
		deviceContext_p->DrawIndexed(testmesh_p->indexCount, 0, 0);

		DirectXViewer::Present();
	}
	void Cleanup()
	{
		// TODO: Add cleanup code here
		delete testmesh_p;


		DirectXViewer::Cleanup();
	}
#pragma endregion

}
