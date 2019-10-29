#pragma once

#include "DirectXViewer.h"


namespace Interface
{
	HRESULT Init(HWND* _hWnd_p);

	void Update();

	void Cleanup();
}