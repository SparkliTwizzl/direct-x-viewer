#pragma once

#include <cstdint>
#include <Windows.h>

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")



namespace DirectXViewer
{
	HRESULT Init(HWND*);
}
