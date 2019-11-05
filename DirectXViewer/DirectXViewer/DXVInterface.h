#pragma once

#include "debug.h"

#include "DirectXViewer.h"


namespace DXVInterface
{
	// Initializes DXV and project-specific resources
	HRESULT Init(HWND* _hWnd_p);

	// Updates DXV and project-specific resources and draws the scene
	void Update(const MSG* _msg);

	// Clears any used heap memory
	void Cleanup();
}