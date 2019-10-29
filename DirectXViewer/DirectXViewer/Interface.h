#pragma once

#include "DirectXViewer.h"


namespace Interface
{
	// Initializes DXV and project-specific resources
	HRESULT Init(HWND* _hWnd_p);

	// Updates DXV and project-specific resources and draws the scene
	void Update();

	// Clears any used heap memory
	void Cleanup();
}