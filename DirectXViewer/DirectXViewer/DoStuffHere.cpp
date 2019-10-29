#include "Interface.h"


namespace Interface
{
	HRESULT Init(HWND* _hWnd_p)
	{
		HRESULT hr;

		hr = DirectXViewer::Init(_hWnd_p);
		if (FAILED(hr)) return hr;

		return hr;
	}

	void Update()
	{
		DirectXViewer::Update();



		DirectXViewer::Draw();
	}

	void Cleanup()
	{
		

		DirectXViewer::Cleanup();
	}

}
