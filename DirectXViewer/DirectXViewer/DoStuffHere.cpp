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
	DirectXViewer::DXVANIMATIONDATA*	testanimdata_p = nullptr;
	DirectXViewer::DXVANIMATION*		testanim_p = nullptr;

	DirectXViewer::DXVOBJECTDATA		testobjdata = {
		testmesh_filename,
		testmat_filename,
		testanim_filename,
		&testmeshdata_p,
		&testmesh_p,
		&testmatdata_p,
		&testmat_p,
		&testanimdata_p,
		&testanim_p
	};
	DirectXViewer::DXVOBJECT			testobj;
#pragma endregion

#pragma region Camera Input Variables
#define INPUT_CAM_TRANS_X_NEG	'A'
#define INPUT_CAM_TRANS_X_POS	'D'
#define INPUT_CAM_TRANS_Y_NEG	VK_SHIFT
#define INPUT_CAM_TRANS_Y_POS	VK_SPACE
#define INPUT_CAM_TRANS_Z_NEG	'S'
#define INPUT_CAM_TRANS_Z_POS	'W'

	struct INPUTS_CAMERA
	{
		enum
		{
			TRANSLATE_X_NEG = 0
			, TRANSLATE_X_POS
			, TRANSLATE_Y_NEG
			, TRANSLATE_Y_POS
			, TRANSLATE_Z_NEG
			, TRANSLATE_Z_POS
			, ROTATE
			, COUNT
		};
	};

	std::bitset<INPUTS_CAMERA::COUNT>	inputs_camera;

	int32_t								xMouse;
	int32_t								yMouse;
	int32_t								xMouse_prev;
	int32_t								yMouse_prev;

	const float							cam_TranslationSpeed = 3.0f;
	const float							cam_RotationSpeed = 0.15f;
#pragma endregion

#pragma region Animation Input Variables
#define INPUT_ANIM_STEP_FWD		VK_OEM_PERIOD
#define INPUT_ANIM_STEP_BCK		VK_OEM_COMMA
#define INPUT_ANIM_PLAY			VK_RIGHT
#define INPUT_ANIM_STOP			VK_LEFT
#define INPUT_ANIM_SPD_UP		VK_UP
#define INPUT_ANIM_SPD_DOWN		VK_DOWN
#define INPUT_ANIM_SPD_RESET	'0'

	struct INPUTS_ANIMATION
	{
		enum
		{
			STEP_FWD = 0
			, STEP_BCK
			, PLAY
			, STOP
			, SPD_UP
			, SPD_DOWN
			, SPD_RESET
			, COUNT
		};
	};

	std::bitset<INPUTS_ANIMATION::COUNT>	inputs_animation;

	const float								inputs_anim_cooldown_length = 0.15f;
	float									inputs_anim_cooldown = 0.0f;

	int32_t									anim_currFrame = 0;
	const float								anim_playbackSpeedStepAmount = 0.1f;
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
	void _ReadCameraInputs(const MSG* _msg)
	{
		// camera translation
		if (_msg->message == WM_KEYDOWN || _msg->message == WM_KEYUP)
		{
			bool value = _msg->message == WM_KEYDOWN;

			switch (_msg->wParam)
			{
			case INPUT_CAM_TRANS_X_NEG:
				inputs_camera.set(INPUTS_CAMERA::TRANSLATE_X_NEG, value);
				break;
			case INPUT_CAM_TRANS_X_POS:
				inputs_camera.set(INPUTS_CAMERA::TRANSLATE_X_POS, value);
				break;
			case INPUT_CAM_TRANS_Y_NEG:
				inputs_camera.set(INPUTS_CAMERA::TRANSLATE_Y_NEG, value);
				break;
			case INPUT_CAM_TRANS_Y_POS:
				inputs_camera.set(INPUTS_CAMERA::TRANSLATE_Y_POS, value);
				break;
			case INPUT_CAM_TRANS_Z_NEG:
				inputs_camera.set(INPUTS_CAMERA::TRANSLATE_Z_NEG, value);
				break;
			case INPUT_CAM_TRANS_Z_POS:
				inputs_camera.set(INPUTS_CAMERA::TRANSLATE_Z_POS, value);
				break;
			default:
				break;
			}
		}

		// camera rotation
		if (_msg->message == WM_RBUTTONDOWN)
		{
			inputs_camera.set(INPUTS_CAMERA::ROTATE, true);
		}
		if (_msg->message == WM_RBUTTONUP)
		{
			inputs_camera.set(INPUTS_CAMERA::ROTATE, false);
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

		float t_camTranslationSpeed = cam_TranslationSpeed * _dt
			, t_camRotationSpeed = cam_RotationSpeed * _dt;

		float dX = 0.0f
			, dY = 0.0f
			, dZ = 0.0f
			, drX = 0.0f
			, drY = 0.0f;


		// get translation amounts

		// X
		if (inputs_camera.test(INPUTS_CAMERA::TRANSLATE_X_NEG))
			dX -= t_camTranslationSpeed;
		if (inputs_camera.test(INPUTS_CAMERA::TRANSLATE_X_POS))
			dX += t_camTranslationSpeed;

		// Y
		if (inputs_camera.test(INPUTS_CAMERA::TRANSLATE_Y_NEG))
			dY -= t_camTranslationSpeed;
		if (inputs_camera.test(INPUTS_CAMERA::TRANSLATE_Y_POS))
			dY += t_camTranslationSpeed;

		// Z
		if (inputs_camera.test(INPUTS_CAMERA::TRANSLATE_Z_NEG))
			dZ -= t_camTranslationSpeed;
		if (inputs_camera.test(INPUTS_CAMERA::TRANSLATE_Z_POS))
			dZ += t_camTranslationSpeed;


		// apply translations
		mView = ((XMMatrixTranslation(dX, 0, 0) * mView) * XMMatrixTranslation(0, dY, 0)) * XMMatrixTranslationFromVector(XMVector3Cross(mView.r[0], { 0, 1, 0 }) * dZ);


		if (inputs_camera.test(INPUTS_CAMERA::ROTATE))
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
	void _ReadAnimationInputs(const MSG* _msg)
	{
		// set animation inputs active if pressed
		if (_msg->message == WM_KEYDOWN)
		{
			switch (_msg->wParam)
			{
			case INPUT_ANIM_STEP_FWD:
				inputs_animation.set(INPUTS_ANIMATION::STEP_FWD, true);
				break;
			case INPUT_ANIM_STEP_BCK:
				inputs_animation.set(INPUTS_ANIMATION::STEP_BCK, true);
				break;
			case INPUT_ANIM_PLAY:
				inputs_animation.set(INPUTS_ANIMATION::PLAY, true);
				break;
			case INPUT_ANIM_STOP:
				inputs_animation.set(INPUTS_ANIMATION::STOP, true);
				break;
			case INPUT_ANIM_SPD_UP:
				inputs_animation.set(INPUTS_ANIMATION::SPD_UP, true);
				break;
			case INPUT_ANIM_SPD_DOWN:
				inputs_animation.set(INPUTS_ANIMATION::SPD_DOWN, true);
				break;
			case INPUT_ANIM_SPD_RESET:
				inputs_animation.set(INPUTS_ANIMATION::SPD_RESET, true);
				break;
			default:
				break;
			}
		}
		// set animation inputs inactive if not pressed
		else if (_msg->message == WM_KEYUP)
		{
			switch (_msg->wParam)
			{
			case INPUT_ANIM_STEP_FWD:
				inputs_animation.set(INPUTS_ANIMATION::STEP_FWD, false);
				break;
			case INPUT_ANIM_STEP_BCK:
				inputs_animation.set(INPUTS_ANIMATION::STEP_BCK, false);
				break;
			case INPUT_ANIM_PLAY:
				inputs_animation.set(INPUTS_ANIMATION::PLAY, false);
				break;
			case INPUT_ANIM_STOP:
				inputs_animation.set(INPUTS_ANIMATION::STOP, false);
				break;
			case INPUT_ANIM_SPD_UP:
				inputs_animation.set(INPUTS_ANIMATION::SPD_UP, false);
				break;
			case INPUT_ANIM_SPD_DOWN:
				inputs_animation.set(INPUTS_ANIMATION::SPD_DOWN, false);
				break;
			case INPUT_ANIM_SPD_RESET:
				inputs_animation.set(INPUTS_ANIMATION::SPD_RESET, false);
				break;
			default:
				break;
			}
		}

		// reset cooldown if no inputs are pressed
		if (!(
			inputs_animation.test(INPUTS_ANIMATION::STEP_FWD)
			|| inputs_animation.test(INPUTS_ANIMATION::STEP_BCK)
			|| inputs_animation.test(INPUTS_ANIMATION::PLAY)
			|| inputs_animation.test(INPUTS_ANIMATION::STOP)
			|| inputs_animation.test(INPUTS_ANIMATION::SPD_UP)
			|| inputs_animation.test(INPUTS_ANIMATION::SPD_DOWN)
			|| inputs_animation.test(INPUTS_ANIMATION::SPD_RESET)
			))
			inputs_anim_cooldown = 0.0f;
	}
	void _UpdateAnimation()
	{
		if (inputs_anim_cooldown <= 0.0f)
		{
			// step animation forward one frame
			if (inputs_animation.test(INPUTS_ANIMATION::STEP_FWD))
			{
				inputs_anim_cooldown = inputs_anim_cooldown_length;
				anim_currFrame++;
			}
			// step animation backward one frame
			if (inputs_animation.test(INPUTS_ANIMATION::STEP_BCK))
			{
				inputs_anim_cooldown = inputs_anim_cooldown_length;
				anim_currFrame--;
			}
			// play animation
			if (inputs_animation.test(INPUTS_ANIMATION::PLAY))
			{
				inputs_anim_cooldown = inputs_anim_cooldown_length;
				testobj.anim_playing = true;
			}
			// stop animation
			if (inputs_animation.test(INPUTS_ANIMATION::STOP))
			{
				inputs_anim_cooldown = inputs_anim_cooldown_length;
				testobj.anim_playing = false;
			}
			// increase animation playback speed
			if (inputs_animation.test(INPUTS_ANIMATION::SPD_UP))
			{
				inputs_anim_cooldown = inputs_anim_cooldown_length;
				testanim_p->playback_speed += anim_playbackSpeedStepAmount;
			}
			// decrease animation playback speed
			if (inputs_animation.test(INPUTS_ANIMATION::SPD_DOWN))
			{
				inputs_anim_cooldown = inputs_anim_cooldown_length;
				testanim_p->playback_speed -= anim_playbackSpeedStepAmount;
				if (testanim_p->playback_speed < anim_playbackSpeedStepAmount)
					testanim_p->playback_speed = anim_playbackSpeedStepAmount;
			}
			// reset animation playback speed to 1.0
			if (inputs_animation.test(INPUTS_ANIMATION::SPD_RESET))
			{
				inputs_anim_cooldown = inputs_anim_cooldown_length;
				testanim_p->playback_speed = 1.0f;
			}
		}


		// clamp current frame within frames that exist
		while (anim_currFrame < 0)
			anim_currFrame += testanimdata_p->frame_count;

		while (anim_currFrame >= (int32_t)testanimdata_p->frame_count)
			anim_currFrame -= testanimdata_p->frame_count;
	}

	HRESULT ManualInit()
	{
		HRESULT hr;

		DirectXViewer::D3DSetClearColor(clearColor);

		XMVECTOR eye = { 0, 7, 10 };
		XMVECTOR at = { 0, 3, 0 };
		XMVECTOR up = { 0, 1, 0 };
		DirectXViewer::SetDefaultViewMatrix(XMMatrixLookAtLH(eye, at, up));

		hr = DirectXViewer::DXVLoadAndCreateObject(testobjdata, &testobj);
		if (FAILED(hr)) return hr;

		testobj.model_matrix = XMMatrixTranslation(-2, 0, 0);

		DirectXViewer::AddObjectToScene(&testobj);

		return hr;
	}
	void ManualUpdate(const MSG* _msg, float _dt)
	{
		if (inputs_anim_cooldown > 0.0f)
			inputs_anim_cooldown -= _dt;

		_ReadAnimationInputs(_msg);
		_UpdateAnimation();

		if (testobj.anim_playing)
			DirectXViewer::debug_AddSkeletonToDebugRenderer(&testanim_p->bind_pose, &testobj.anim_currFrame, XMMatrixTranslation(2, 0, 0));
		else
			DirectXViewer::debug_AddSkeletonToDebugRenderer(&testanim_p->bind_pose, &testanim_p->frames[anim_currFrame], XMMatrixTranslation(2, 0, 0));
	}
	void ManualDraw()
	{
	}
	void ManualCleanup()
	{
		delete testanim_p;
		delete testanimdata_p;
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

		_ReadCameraInputs(_msg);
		_UpdateCamera(dt);

		// TODO: Add update code here
		ManualUpdate(_msg, dt);

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
