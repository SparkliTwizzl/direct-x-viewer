#include <bitset>

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
	XMFLOAT4							clearColor = GREY_DARK_RGBA_FLOAT;

	const char*							testmesh_filename = "assets/Run.mesh";
	DirectXViewer::DXVMESHDATA*			testmeshdata_p = nullptr;
	DirectXViewer::DXVMESH*				testmesh_p = nullptr;

	const char*							testmat_filename = "assets/Run.mat";
	DirectXViewer::DXVMATERIALDATA*		testmatdata_p = nullptr;
	DirectXViewer::DXVMATERIAL*			testmat_p = nullptr;

	const char*							testanim_filename = "assets/Run.anim";
	/*
	28 joints
	0.708333 duration
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

#pragma region Input Variables
#define INPUT_X_NEG 'A'
#define INPUT_X_POS 'D'
#define INPUT_Y_NEG VK_SHIFT
#define INPUT_Y_POS VK_SPACE
#define INPUT_Z_NEG 'S'
#define INPUT_Z_POS 'W'

	enum Inputs_e
	{
		CamTranslateXNeg = 0
		, CamTranslateXPos
		, CamTranslateYNeg
		, CamTranslateYPos
		, CamTranslateZNeg
		, CamTranslateZPos
		, CamRotate
		, Count
	};

	std::bitset<Inputs_e::Count>	inputValues;

	int32_t							xMouse;
	int32_t							yMouse;
	int32_t							xMouse_prev;
	int32_t							yMouse_prev;

	const float						camTranslationSpeed = 4.0f;
	const float						camRotationSpeed = 0.1f;
#pragma endregion


#pragma region Private Helper Functions
	float _GetTimeSinceLastUpdate()
	{
		static uint64_t startTime = 0;
		static uint64_t prevTime = 0;
		uint64_t curTime = GetTickCount64();
		if (startTime == 0)
			startTime = prevTime = curTime;
		static float t = (curTime - startTime) / 1000.0f;
		float dt = (curTime - prevTime) / 1000.0f;
		prevTime = curTime;

		return dt;
	}
	void _ReadInputs(const MSG* _msg)
	{
		// camera translation
		if (_msg->message == WM_KEYDOWN || _msg->message == WM_KEYUP)
		{
			bool value = _msg->message == WM_KEYDOWN;

			switch (_msg->wParam)
			{
			case INPUT_X_NEG:
				inputValues.set(Inputs_e::CamTranslateXNeg, value);
				break;
			case INPUT_X_POS:
				inputValues.set(Inputs_e::CamTranslateXPos, value);
				break;
			case INPUT_Y_NEG:
				inputValues.set(Inputs_e::CamTranslateYNeg, value);
				break;
			case INPUT_Y_POS:
				inputValues.set(Inputs_e::CamTranslateYPos, value);
				break;
			case INPUT_Z_NEG:
				inputValues.set(Inputs_e::CamTranslateZNeg, value);
				break;
			case INPUT_Z_POS:
				inputValues.set(Inputs_e::CamTranslateZPos, value);
				break;
			default:
				break;
			}
		}

		// camera rotation
		if (_msg->message == WM_RBUTTONDOWN)
		{
			inputValues.set(Inputs_e::CamRotate, true);
		}
		if (_msg->message == WM_RBUTTONUP)
		{
			inputValues.set(Inputs_e::CamRotate, false);
		}
		if (_msg->message == WM_MOUSEMOVE)
		{
			xMouse_prev = xMouse;
			yMouse_prev = yMouse;
			xMouse = LOWORD(_msg->lParam);
			yMouse = HIWORD(_msg->lParam);
		}
	}
	void _UpdateCamera(float _dt)
	{
		XMMATRIX mView = XMMatrixInverse(nullptr, DirectXViewer::GetDefaultViewMatrix());

		float t_camTranslationSpeed = camTranslationSpeed * _dt
			, t_camRotationSpeed = camRotationSpeed * _dt;

		float dX = 0.0f
			, dY = 0.0f
			, dZ = 0.0f
			, drX = 0.0f
			, drY = 0.0f;


		// get translation amounts

		// X
		if (inputValues.test(Inputs_e::CamTranslateXNeg))
			dX -= t_camTranslationSpeed;
		if (inputValues.test(Inputs_e::CamTranslateXPos))
			dX += t_camTranslationSpeed;

		// Y
		if (inputValues.test(Inputs_e::CamTranslateYNeg))
			dY -= t_camTranslationSpeed;
		if (inputValues.test(Inputs_e::CamTranslateYPos))
			dY += t_camTranslationSpeed;

		// Z
		if (inputValues.test(Inputs_e::CamTranslateZNeg))
			dZ -= t_camTranslationSpeed;
		if (inputValues.test(Inputs_e::CamTranslateZPos))
			dZ += t_camTranslationSpeed;


		// apply translations
		mView = ((XMMatrixTranslation(dX, 0, 0) * mView) * XMMatrixTranslation(0, dY, 0)) * XMMatrixTranslationFromVector(XMVector3Cross(mView.r[0], { 0, 1, 0 }) * dZ);


		if (inputValues.test(Inputs_e::CamRotate))
		{
			// get rotation amounts
			float camRotY = (float)(xMouse - xMouse_prev) * t_camRotationSpeed
				, camRotX = (float)(yMouse - yMouse_prev) * t_camRotationSpeed;

			// get view matrix offset from origin
			XMVECTOR offset = { XMVectorGetX(mView.r[3]), XMVectorGetY(mView.r[3]), XMVectorGetZ(mView.r[3]) };

			// temporarily shift view matrix to origin and apply rotations
			mView = mView * XMMatrixTranslationFromVector(-offset);
			mView = (XMMatrixRotationX(camRotX) * mView) * XMMatrixRotationY(camRotY);
			mView = mView * XMMatrixTranslationFromVector(offset);
		}


		DirectXViewer::SetDefaultViewMatrix(XMMatrixInverse(nullptr, mView));
	}


	// TODO: Add functions here

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

		testobj.model_matrix = XMMatrixTranslation(-2, 0, 0);

		DirectXViewer::AddObjectToScene(&testobj);


		return hr;
	}
	void ManualUpdate()
	{
		DirectXViewer::debug_AddSkeletonToDebugRenderer(&testanim_p->bind_pose, &testanim_p->keyframes[0], XMMatrixTranslation(0, 0, 0));
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
		float dt = _GetTimeSinceLastUpdate();
		DirectXViewer::Update(_msg, dt);

		_ReadInputs(_msg);
		_UpdateCamera(dt);

		// TODO: Add update code here
		ManualUpdate();

		// TODO: Add draw code here
		ManualDraw();

		//uint32_t draw_mode = DirectXViewer::DRAW_MODE::DRAW_DEBUG | DirectXViewer::DRAW_MODE::NO_SCENE_OBJECTS;
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
