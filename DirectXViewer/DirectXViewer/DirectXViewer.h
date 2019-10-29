#pragma once

#include <cstdint>
#include <Windows.h>

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")



namespace DirectXViewer
{
	// Creates and initializes automatically created D3D and DXV resources
	// Additional resources must be created manually after this function is called
	HRESULT Init(HWND*);


	// Creates and stores a D3D11 depth stencil and corresponding depth stencil view
	HRESULT DxCreateDepthStencilView(uint32_t, uint32_t, ID3D11Texture2D**, ID3D11DepthStencilView**);
}
