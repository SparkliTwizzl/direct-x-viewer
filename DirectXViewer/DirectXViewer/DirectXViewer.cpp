#include "DirectXViewer.h"



namespace DirectXViewer
{
	// window
	HWND*						hWnd_p = nullptr;

	uint32_t					windowWidth = 0;
	uint32_t					windowHeight = 0;

	// D3D
	D3D_DRIVER_TYPE				driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL			featureLeve = D3D_FEATURE_LEVEL_11_0;

	ID3D11Device*				device_p = nullptr;
	ID3D11DeviceContext*		deviceContext_p = nullptr;
	IDXGISwapChain*				swapChain_p = nullptr;



	HRESULT Init(HWND* _hWnd_p)
	{
		HRESULT hr = S_OK;

		hWnd_p = _hWnd_p;

		// get window dimensions
		RECT wRect;
		GetClientRect(*hWnd_p, &wRect);
		windowWidth = wRect.right - wRect.left;
		windowHeight = wRect.bottom - wRect.top;


		return hr;
	}

}
