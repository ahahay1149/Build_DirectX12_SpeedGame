#include "MyAppRunner.h"
#include "InputManager.h"

#if DIRECT_INPUT_ACTIVE
//デバイスの検索に使うコールバック、グローバルでないと動かない
LPDIRECTINPUT8			g_pDInput = nullptr;			// IDirectInput8インターフェースへのポインタ
LPDIRECTINPUTDEVICE8	g_pDIGamePad[MAX_DI_PADS];		// IDirectInputDevice8インターフェースへのポインタ
UINT					g_DICount = 0;
//HWND					g_hwnd;

//コールバック関数はCALLBACK宣言が無いと呼び出し方法が異なるので実行時エラーとなる
//EnumDeviceがDirectInputデバイスを見つけたら呼ばれる
BOOL CALLBACK EnumDIJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext)
{
	HRESULT hr;

	//デバイス作成
	hr = g_pDInput->CreateDevice(pdidInstance->guidInstance, &g_pDIGamePad[g_DICount], NULL);
	if (FAILED(hr)) return DIENUM_CONTINUE;	//失敗したらここで終了

	//デバイスタイプ設定
	hr = g_pDIGamePad[g_DICount]->SetDataFormat(&c_dfDIJoystick2);	//このデバイスはジョイスティック（ゲームパッド）
																	//c_dfDIJoystick : ジョイスティック（アナログなし）
																	//c_dfDIKeyboard : キーボード / c_dfDIMouse : マウス
	if (FAILED(hr))
	{
		goto FAILED_POINT;
	}

	//アナログ軸モード設定
	DIPROPDWORD diProp;
	ZeroMemory(&diProp, sizeof(DIPROPDWORD));
	diProp.diph.dwSize = sizeof(DIPROPDWORD);
	diProp.diph.dwHeaderSize = sizeof(diProp.diph);
	diProp.diph.dwHow = DIPH_DEVICE;
	diProp.diph.dwObj = 0;
	diProp.dwData = DIPROPAXISMODE_ABS;
	hr = g_pDIGamePad[g_DICount]->SetProperty(DIPROP_AXISMODE, &diProp.diph);

	if (FAILED(hr))
	{
		goto FAILED_POINT;
	}

	//アナログ軸数値範囲設定
	DIPROPRANGE diPrg;
	ZeroMemory(&diPrg, sizeof(DIPROPRANGE));
	diPrg.diph.dwSize = sizeof(DIPROPRANGE);
	diPrg.diph.dwHeaderSize = sizeof(diPrg.diph);
	diPrg.diph.dwHow = DIPH_BYOFFSET;
	diPrg.diph.dwObj = DIJOFS_X;
	diPrg.lMin = -32767;				//xInput基準
	diPrg.lMax = 32767;
	//X軸
	hr = g_pDIGamePad[g_DICount]->SetProperty(DIPROP_RANGE, &diPrg.diph);
	if (FAILED(hr))
	{
		goto FAILED_POINT;
	}
	//Y軸
	diPrg.diph.dwObj = DIJOFS_Y;
	hr = g_pDIGamePad[g_DICount]->SetProperty(DIPROP_RANGE, &diPrg.diph);
	if (FAILED(hr))
	{
		goto FAILED_POINT;
	}

	//協調モード（Windowが後ろに行っても反応するか、しないか）設定
	//DISCL_EXCLUSIVE : 限定　DISCL_FOREGROUND : Windowがトップ 
	hr = g_pDIGamePad[g_DICount]->SetCooperativeLevel(MyAppRunner::getHwnd(), DISCL_EXCLUSIVE | DISCL_FOREGROUND);
	if (FAILED(hr))
	{
		goto FAILED_POINT;
	}

	//パッド入力開始
	hr = g_pDIGamePad[g_DICount]->Acquire();
	if (FAILED(hr))
	{
		goto FAILED_POINT;
	}

	goto SUCCESS_POINT;		//初期化完了

FAILED_POINT:

	g_pDIGamePad[g_DICount]->Release();
	g_pDIGamePad[g_DICount] = nullptr;
	return DIENUM_CONTINUE;

SUCCESS_POINT:

	g_DICount++;
	if (g_DICount < MAX_DI_PADS)
		return DIENUM_CONTINUE;

	return DIENUM_STOP;	//DIENUM_STOPが来るとこれ以上の列挙を停止する
}
#endif

InputManager::InputManager()
{
	int i, j;
	// 00:押されていない 01:押されている 11:押された瞬間 10:離された瞬間
	for (i = 0; i < MAX_KEYS; i++)
	{
		m_keyState[i] = 0;
	}

	for (i = 0; i < MAX_MOUSE_BUTTONS; i++)
	{
		m_mouseState[i] = 0;
	}

	for (i = 0; i < MAX_PADS; i++)
	{
		m_padActive[i] = false;
		
		for (j = 0; j < MAX_PAD_BUTTONS; j++)
		{
			m_buttonState[i][j] = 0;
		}

		for (j = 0; j < MAX_PAD_ANALOGIN; j++)
		{
			m_analogState[i][j] = 0;
		}
	}

#if DIRECT_INPUT_ACTIVE
	for (i = 0; i < MAX_DI_PADS; i++)
	{
		//POV
		m_diDirection[i][0] = 0;
		m_diDirection[i][1] = 0;
		m_diDirection[i][2] = 0;
		m_diDirection[i][3] = 0;

		//BUTTON
		for (j = 0; j < MAX_DI_BUTTONS; j++)
		{
			m_diButtonState[i][j] = 0;
		}

		//ANALOG
		for (j = 0; j < MAX_DI_ANALOGIN; j++)
		{
			m_diAnalogState[i][j] = 0;
		}
	}
#endif

}

InputManager::~InputManager()
{
	setInputEnable(false);

#if DIRECT_INPUT_ACTIVE
	releaseDirectInput();
#endif
}

void InputManager::update()
{

	//xInputのポーリング（入力データをXINPUT_STATEに取得）
	DWORD dwResult;
	XINPUT_STATE xstate;
	int i;
	for (i = 0; i < MAX_PADS; i++)
	{
		// Simply get the state of the controller from XInput.
		ZeroMemory(&xstate, sizeof(XINPUT_STATE));
		dwResult = XInputGetState(i, &xstate);

		if (dwResult == ERROR_SUCCESS)
		{
			m_padActive[i] = true;

			//スティックのデッドゾーン反映
			// Zero value if thumbsticks are within the dead zone 
			if ((xstate.Gamepad.sThumbLX < INPUT_DEADZONE_L &&
				xstate.Gamepad.sThumbLX > -INPUT_DEADZONE_L) &&
				(xstate.Gamepad.sThumbLY < INPUT_DEADZONE_L &&
					xstate.Gamepad.sThumbLY > -INPUT_DEADZONE_L))
			{
				xstate.Gamepad.sThumbLX = 0;
				xstate.Gamepad.sThumbLY = 0;
			}

			if ((xstate.Gamepad.sThumbRX < INPUT_DEADZONE_R &&
				xstate.Gamepad.sThumbRX > -INPUT_DEADZONE_R) &&
				(xstate.Gamepad.sThumbRY < INPUT_DEADZONE_R &&
					xstate.Gamepad.sThumbRY > -INPUT_DEADZONE_R))
			{
				xstate.Gamepad.sThumbRX = 0;
				xstate.Gamepad.sThumbRY = 0;
			}

			//btn更新
			setXButton(i, (int)X_BUTTONS::DPAD_UP, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP));
			setXButton(i, (int)X_BUTTONS::DPAD_DOWN, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN));
			setXButton(i, (int)X_BUTTONS::DPAD_LEFT, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT));
			setXButton(i, (int)X_BUTTONS::DPAD_RIGHT, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT));
			setXButton(i, (int)X_BUTTONS::BTN_START, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_START));
			setXButton(i, (int)X_BUTTONS::BTN_BACK, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK));
			setXButton(i, (int)X_BUTTONS::BTN_L3, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB));
			setXButton(i, (int)X_BUTTONS::BTN_R3, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB));
			setXButton(i, (int)X_BUTTONS::BTN_L, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER));
			setXButton(i, (int)X_BUTTONS::BTN_R, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER));

			setXButton(i, (int)X_BUTTONS::BTN_GUIDE, (xstate.Gamepad.wButtons & 0x0400));
			setXButton(i, (int)X_BUTTONS::BTN_UNKNOWN, (xstate.Gamepad.wButtons & 0x0800));

			setXButton(i, (int)X_BUTTONS::BTN_A, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_A));
			setXButton(i, (int)X_BUTTONS::BTN_B, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_B));
			setXButton(i, (int)X_BUTTONS::BTN_X, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_X));
			setXButton(i, (int)X_BUTTONS::BTN_Y, (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_Y));

			//アナログスティック更新
			m_analogState[i][(int)X_ANALOGS::L_X]= xstate.Gamepad.sThumbLX;
			m_analogState[i][(int)X_ANALOGS::L_Y] = xstate.Gamepad.sThumbLY;
			m_analogState[i][(int)X_ANALOGS::R_X] = xstate.Gamepad.sThumbRX;
			m_analogState[i][(int)X_ANALOGS::R_Y] = xstate.Gamepad.sThumbRY;
			m_analogState[i][(int)X_ANALOGS::L_TRIGGER] = xstate.Gamepad.bLeftTrigger;
			m_analogState[i][(int)X_ANALOGS::R_TRIGGER] = xstate.Gamepad.bRightTrigger;

		}
		else
		{
			if (m_padActive[i])
			{
				//btnリセット
				ZeroMemory(&m_buttonState[i], MAX_PAD_BUTTONS);
				m_padActive[i] = false;
			}
		}
	}

#ifdef DIRECT_INPUT_ACTIVE
	DIJOYSTATE2 jstate;	//DIフォースフィードバック等、対応版
	int j;

	//DirectInputのポーリング
	// 失敗したら削除するだけ
	for (i = 0; i < MAX_DI_PADS; i++)
	{
		if (g_pDIGamePad[i])
		{
			g_pDIGamePad[i]->Poll();	//POLLも失敗する可能性はあるものの、それなら次で吸収

			if (FAILED(g_pDIGamePad[i]->GetDeviceState(sizeof(DIJOYSTATE2), &jstate)))
			{
				//再起動を試みる
				if (FAILED(g_pDIGamePad[i]->Acquire()))
				{
					releaseDIGameController(i);
				}
				else
				{
					g_pDIGamePad[i]->Poll();
				}

				//ここからDeviceState取っても中身はないので次フレームへ
			}
			else
			{
				if ((jstate.lX < INPUT_DEADZONE_L &&
					jstate.lX > -INPUT_DEADZONE_L) &&
					(jstate.lY < INPUT_DEADZONE_L &&
						jstate.lY > -INPUT_DEADZONE_L) &&
					(jstate.lZ < INPUT_DEADZONE_L &&
						jstate.lZ > -INPUT_DEADZONE_L))
				{
					jstate.lX = 0;
					jstate.lY = 0;
					jstate.lZ = 0;
				}

				if ((jstate.lRx < INPUT_DEADZONE_R &&
					jstate.lRx > -INPUT_DEADZONE_R) &&
					(jstate.lRy < INPUT_DEADZONE_R &&
						jstate.lRy > -INPUT_DEADZONE_R) &&
					(jstate.lRz < INPUT_DEADZONE_R &&
						jstate.lRz > -INPUT_DEADZONE_R))
				{
					jstate.lRx = 0;
					jstate.lRy = 0;
					jstate.lRz = 0;
				}

				for (j = 0; j < MAX_DI_BUTTONS; j++)
				{
					setXButton(MAX_PADS + i, j, (jstate.rgbButtons[i] & 0x80));
				}

				for (j = 0; j < 4; j++)
				{
					m_diDirection[i][j] &= 0xf0;
					switch (jstate.rgdwPOV[j])
					{
					case -1:	//押してない
						break;

					case 0:		//上
						m_diDirection[i][j] |= (BYTE)DI_POV::DPOV_UP;
						break;

					case 4500:	//右上
						m_diDirection[i][j] |= (BYTE)(DI_POV::DPOV_UP) | (BYTE)(DI_POV::DPOV_RIGHT);
						break;

					case 9000:	//右
						m_diDirection[i][j] |= (BYTE)DI_POV::DPOV_RIGHT;
						break;

					case 13500:	//右下
						m_diDirection[i][j] |= (BYTE)(DI_POV::DPOV_DOWN) | (BYTE)(DI_POV::DPOV_RIGHT);
						break;

					case 18000:	//下
						m_diDirection[i][j] |= (BYTE)DI_POV::DPOV_DOWN;
						break;

					case 22500:	//左下
						m_diDirection[i][j] |= (BYTE)(DI_POV::DPOV_DOWN) | (BYTE)(DI_POV::DPOV_LEFT);
						break;

					case 27000:	//左
						m_diDirection[i][j] |= (BYTE)DI_POV::DPOV_LEFT;
						break;

					case 31500:	//左上
						m_diDirection[i][j] |= (BYTE)(DI_POV::DPOV_UP) | (BYTE)(DI_POV::DPOV_LEFT);
						break;
					}
				}

				//アナログIN保存。
				m_diAnalogState[i][(int)DI_ANALOGS::L_X] = jstate.lX;
				m_diAnalogState[i][(int)DI_ANALOGS::L_Y] = jstate.lY;
				m_diAnalogState[i][(int)DI_ANALOGS::L_Z] = jstate.lZ;

				m_diAnalogState[i][(int)DI_ANALOGS::R_X] = jstate.lRx;
				m_diAnalogState[i][(int)DI_ANALOGS::R_Y] = jstate.lRy;
				m_diAnalogState[i][(int)DI_ANALOGS::R_Z] = jstate.lRz;

				m_diAnalogState[i][(int)DI_ANALOGS::LA_X] = jstate.lAX;
				m_diAnalogState[i][(int)DI_ANALOGS::LA_Y] = jstate.lAY;
				m_diAnalogState[i][(int)DI_ANALOGS::LA_Z] = jstate.lAZ;

				m_diAnalogState[i][(int)DI_ANALOGS::SLIDER_X] = jstate.rglSlider[0];
				m_diAnalogState[i][(int)DI_ANALOGS::SLIDER_Y] = jstate.rglSlider[1];

				m_diAnalogState[i][(int)DI_ANALOGS::V_SLIDER_X] = jstate.rglVSlider[0];
				m_diAnalogState[i][(int)DI_ANALOGS::V_SLIDER_Y] = jstate.rglVSlider[1];

				m_diAnalogState[i][(int)DI_ANALOGS::F_SLIDER_X] = jstate.rglFSlider[0];
				m_diAnalogState[i][(int)DI_ANALOGS::F_SLIDER_Y] = jstate.rglFSlider[1];

				m_diAnalogState[i][(int)DI_ANALOGS::A_SLIDER_X] = jstate.rglASlider[0];
				m_diAnalogState[i][(int)DI_ANALOGS::A_SLIDER_Y] = jstate.rglASlider[1];

				m_diAnalogState[i][(int)DI_ANALOGS::LAR_X] = jstate.lARx;
				m_diAnalogState[i][(int)DI_ANALOGS::LAR_Y] = jstate.lARy;
				m_diAnalogState[i][(int)DI_ANALOGS::LAR_Z] = jstate.lARz;

				m_diAnalogState[i][(int)DI_ANALOGS::LF_X] = jstate.lFX;
				m_diAnalogState[i][(int)DI_ANALOGS::LF_Y] = jstate.lFY;
				m_diAnalogState[i][(int)DI_ANALOGS::LF_Z] = jstate.lFZ;

				m_diAnalogState[i][(int)DI_ANALOGS::LFR_X] = jstate.lFRx;
				m_diAnalogState[i][(int)DI_ANALOGS::LFR_Y] = jstate.lFRy;
				m_diAnalogState[i][(int)DI_ANALOGS::LFR_Z] = jstate.lFRz;

				m_diAnalogState[i][(int)DI_ANALOGS::LV_X] = jstate.lVX;
				m_diAnalogState[i][(int)DI_ANALOGS::LV_Y] = jstate.lVY;
				m_diAnalogState[i][(int)DI_ANALOGS::LV_Z] = jstate.lVZ;

				m_diAnalogState[i][(int)DI_ANALOGS::LVR_X] = jstate.lVRx;
				m_diAnalogState[i][(int)DI_ANALOGS::LVR_Y] = jstate.lVRy;
				m_diAnalogState[i][(int)DI_ANALOGS::LVR_Z] = jstate.lVRz;

			}
		}
	}
#endif
}

void InputManager::refreshBuffer()
{
	int i, j;
	for (i = 0; i < MAX_KEYS; i++)
	{
		m_keyState[i] &= 0x01;	//Pressのフラグだけ残す
	}

	for (i = 0; i < MAX_MOUSE_BUTTONS; i++)
	{
		m_mouseState[i] &= 0x01;
	}

	for (i = 0; i < MAX_PADS; i++)
	{
		for (j = 0; j < MAX_PAD_ANALOGIN; j++)
		{
			m_analogState[i][j] = 0;
		}

		for (j = 0; j < MAX_PAD_BUTTONS; j++)
		{
			m_buttonState[i][j] &= 0x01;
		}
	}

#if DIRECT_INPUT_ACTIVE
	for (i = 0; i < MAX_DI_PADS; i++)
	{
		for (j = 0; j < MAX_PAD_ANALOGIN; j++)
		{
			m_diAnalogState[i][j] = 0;
		}

		//前フレームのPUSHを4bit左にシフト
		m_diDirection[i][0] <<= 4;
		m_diDirection[i][1] <<= 4;
		m_diDirection[i][2] <<= 4;
		m_diDirection[i][3] <<= 4;

		for (j = 0; j < MAX_DI_BUTTONS; j++)
		{
			m_diButtonState[i][j] &= 0x01;
		}
	}
#endif
}

void InputManager::keyPress(WPARAM keycode)
{
	if (keycode >= MAX_KEYS)
		return;

	//キーボードでの初回チェックが出来てなかったので処理変更

	BYTE flg = m_keyState[keycode];
	BYTE stat = flg & 0x3;			//下位2bitが欲しい
	stat ^= 0x01;	//最下位bitに１をxor
	stat <<= 1;		//左に1bitシフト
	stat |= 0x01;	//最下位bitを1

	//下位2bitを入れ替え
	m_keyState[keycode] = (flg & 0xfc) | stat;
}

void InputManager::keyUp(WPARAM keycode)
{
	if (keycode >= MAX_KEYS)
		return;

	m_keyState[keycode] = 0x02;
}

void InputManager::mouseButtonDown(UINT buttonId)
{
	if (buttonId >= MAX_MOUSE_BUTTONS)
		return;

	m_mouseState[buttonId] = 0x03;
}

void InputManager::mouseButtonUp(UINT buttonId)
{
	if (buttonId >= MAX_MOUSE_BUTTONS)
		return;

	m_mouseState[buttonId] = 0x02;
}

void InputManager::mouseMove(short x, short y)
{
	m_mousePos.x = x;
	m_mousePos.y = y;
}

void InputManager::mouseWheel(short delta, short x, short y)
{
	m_wheelPos.x = delta * x;
	m_wheelPos.y = delta * y;
}

bool InputManager::checkKeyboard(UINT keycode, BUTTON_STATE state)
{
	if (keycode >= MAX_KEYS)
		return false;

	return checkState(m_keyState[keycode], state);
}

bool InputManager::checkGamePad(UINT padId, UINT btnId, BUTTON_STATE state)
{
	if (padId < MAX_PADS && m_padActive[padId])
	{
		if (btnId < MAX_PAD_BUTTONS)
			return checkState(m_buttonState[padId][btnId], state);

		return false;
	}
	else
	{
		padId -= MAX_PADS;
		if (padId < MAX_DI_PADS && g_pDIGamePad[padId])
		{
			if (btnId < MAX_DI_BUTTONS)
				return checkState(m_diButtonState[padId][btnId], state);
		}

		return false;
	}
}

bool InputManager::checkMouseButton(UINT btnId, BUTTON_STATE state)
{
	if (btnId >= MAX_MOUSE_BUTTONS)
		return false;

	return checkState(m_mouseState[btnId], state);
}

XMINT2 InputManager::getMousePosition()
{
	return m_mousePos;
}

XMINT2 InputManager::getWheel()
{
	return m_wheelPos;
}

INT InputManager::getAnalogValue(UINT padId, int analogId)
{
	INT res = 0;

	if (padId < MAX_PADS && analogId < MAX_PAD_ANALOGIN && m_padActive[padId])
	{
		return m_analogState[padId][analogId];
	}

	return res;
}

void InputManager::setInputEnable(bool flg)
{
	if (flg)
	{
#if DIRECT_INPUT_ACTIVE
#endif
	}
	else
	{
		//振動オフ
		XINPUT_VIBRATION vib = {};

		for (int i = 0; i < MAX_PADS; i++)
			XInputSetState(i, &vib);
	}
}

#if DIRECT_INPUT_ACTIVE

HRESULT InputManager::initDirectInput(HINSTANCE hInst)
{

	HRESULT hr;
	if (!g_pDInput)
	{
		// DirectInputオブジェクトの作成
		hr = DirectInput8Create(hInst, DIRECTINPUT_VERSION,
			IID_IDirectInput8, (void**)&g_pDInput, NULL);

		if (FAILED(hr))
		{
			return hr;
		}
	}

	//起動前にコントローラを繋げてないと自動的にONにはできない
	hr = g_pDInput->EnumDevices(DI8DEVCLASS_GAMECTRL, (LPDIENUMDEVICESCALLBACKW)EnumDIJoysticksCallback,
		NULL, DIEDFL_ATTACHEDONLY);
	
	return hr;
}

//削除
void InputManager::releaseDirectInput()
{
	releaseAllDIControllers();
	if (g_pDInput)
	{
		g_pDInput->Release();
		g_pDInput = nullptr;
	}
}

bool InputManager::checkDirectionButton(UINT padId, UINT povId, X_BUTTONS btnId, BUTTON_STATE state)
{
	if (padId < MAX_PADS && m_padActive[padId])
	{
		switch (btnId)
		{
		case X_BUTTONS::DPAD_UP:
		case X_BUTTONS::DPAD_RIGHT:
		case X_BUTTONS::DPAD_DOWN:
		case X_BUTTONS::DPAD_LEFT:
			return checkState(m_buttonState[padId][(UINT)btnId], state);
		}
	}
	else
	{
		if (povId < 4)
		{
			padId -= MAX_PADS;

			if (padId < MAX_DI_PADS && g_pDIGamePad[padId])
			{
				BYTE flg = m_diDirection[padId][povId];
				BYTE mask = 0;

				switch (btnId)
				{
				case X_BUTTONS::DPAD_UP:
					mask = (BYTE)DI_POV::DPOV_UP;
					break;
				case X_BUTTONS::DPAD_RIGHT:
					mask = (BYTE)DI_POV::DPOV_RIGHT;
					break;
				case X_BUTTONS::DPAD_DOWN:
					mask = (BYTE)DI_POV::DPOV_DOWN;
					break;
				case X_BUTTONS::DPAD_LEFT:
					mask = (BYTE)DI_POV::DPOV_LEFT;
					break;
				}

				switch (state)
				{
				case BUTTON_STATE::BUTTON_UP:
					mask = mask | (mask << 4);
					return (flg & mask) == (0xf0 & mask);	//下のビットだけが0

				case BUTTON_STATE::BUTTON_DOWN:
					mask = mask | (mask << 4);
					return (flg & mask) == (0x0f & mask);	//下のビットだけが1

				case BUTTON_STATE::BUTTON_PRESS:			//下のビットが1
					return (flg & mask);
				}
			}
		}
	}

	return false;
}

LONG InputManager::getDiAnalogValue(UINT padId, int analogId)
{
	LONG res = 0;

	if (padId > MAX_PADS)
	{
		padId -= MAX_PADS;

		if (padId < MAX_DI_PADS)
		{
			if (analogId < MAX_DI_ANALOGIN)
			{
				return m_diAnalogState[padId][analogId];
			}
		}
	}

	return res;
}

//protected access
void InputManager::releaseDIGameController(int conId)
{
	if (conId < MAX_DI_PADS)
	{
		if (g_pDIGamePad[conId])
		{
			g_pDIGamePad[conId]->Unacquire();
			g_pDIGamePad[conId]->Release();
			g_pDIGamePad[conId] = nullptr;
		}
	}
}

void InputManager::releaseAllDIControllers()
{
	for (int i = 0; i < MAX_DI_PADS; i++)
	{
		releaseDIGameController(i);
	}

	g_DICount = 0;
}

#endif

void InputManager::setXButton(UINT padId, UINT btnId, bool push)
{
	BYTE* targetState = nullptr;
	if (padId < MAX_PADS)
	{
		targetState = m_buttonState[padId];
	}
	else
	{
		padId -= MAX_PADS;
		if (padId < MAX_DI_PADS)
		{
			targetState = m_diButtonState[padId];
		}
		else
		{
			return;
		}
	}

	if (push)
	{
		//press or down
		if (targetState[btnId] != 0)
		{
			targetState[btnId] = 0x01;	//down	押し続けられている
		}
		else
		{
			targetState[btnId] = 0x03;	//press	押された瞬間
		}
	}
	else
	{
		//up
		if (targetState[btnId] != 0)
		{
			targetState[btnId] = 0x02;	//up	離された瞬間
		}

		//0 押されていない
	}

}

bool InputManager::checkState(UINT flg, BUTTON_STATE state)
{
	switch (state)
	{
	case BUTTON_STATE::BUTTON_DOWN:
		return (flg == 0x03);
	case BUTTON_STATE::BUTTON_PRESS:
		return ((flg & 0x01) == 0x01);
	case BUTTON_STATE::BUTTON_UP:
		return (flg == 0x02);
	}
	return false;
}
