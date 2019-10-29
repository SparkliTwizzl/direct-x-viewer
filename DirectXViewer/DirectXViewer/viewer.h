#pragma once

#include "DirectXViewer.h"


namespace viewer
{
	HRESULT Init(HWND* _hWnd_p);

	void Update();

	void Cleanup();
}