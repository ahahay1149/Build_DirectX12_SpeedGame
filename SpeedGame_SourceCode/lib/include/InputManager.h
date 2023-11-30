#pragma once

#include <Windows.h>		//xinputのinclude前にWindows.hのincludeがないとビルド失敗するケース有り
#include <directxmath.h>
#include <Xinput.h>

#pragma comment(lib, "xinput.lib")

#define DIRECT_INPUT_ACTIVE 1

#if DIRECT_INPUT_ACTIVE
#include <dinput.h>
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dinput8.lib")

#define MAX_DI_PADS 4			//とりあえず4個
#define MAX_DI_BUTTONS 32		//とりあえず32個
#define MAX_DI_ANALOGIN 32
#endif


#define MAX_KEYS 256
#define MAX_MOUSE_BUTTONS 5
#define MAX_PAD_BUTTONS 16		//上下左右＋10ボタンまで対応
#define MAX_PAD_ANALOGIN 8		//アナログ入力の数
#define MAX_PADS 4				//xInputのパッドは4つまで

#define INPUT_DEADZONE_L  ( 0.24f * FLOAT(0x7FFF) )  // Default to 24% of the +/- 32767 range.   This is a reasonable default value but can be altered if needed.
#define INPUT_DEADZONE_R  ( 0.24f * FLOAT(0x7FFF) )  // Default to 24% of the +/- 32767 range.   This is a reasonable default value but can be altered if needed.

using namespace DirectX;

class InputManager
{
public:

	enum class BUTTON_STATE
	{
		BUTTON_DOWN,
		BUTTON_PRESS,
		BUTTON_UP
	};

	enum class X_BUTTONS
	{
		DPAD_UP = 0,
		DPAD_DOWN,
		DPAD_LEFT,
		DPAD_RIGHT,
		BTN_START,
		BTN_BACK,
		BTN_L3,
		BTN_R3,
		BTN_L,
		BTN_R,

		BTN_GUIDE,
		BTN_UNKNOWN,

		BTN_A,
		BTN_B,
		BTN_X,
		BTN_Y,
	};

	enum class X_ANALOGS
	{
		L_X = 0,
		L_Y,
		L_Z,
		R_X,
		R_Y,
		R_Z,
		L_TRIGGER,
		R_TRIGGER,
	};

	enum class DI_ANALOGS
	{
		L_X = 0,
		L_Y,
		L_Z,
		R_X,
		R_Y,
		R_Z,

		SLIDER_X,
		SLIDER_Y,
		V_SLIDER_X,
		V_SLIDER_Y,
		A_SLIDER_X,
		A_SLIDER_Y,
		F_SLIDER_X,
		F_SLIDER_Y,

		LA_X,
		LA_Y,
		LA_Z,
		LAR_X,
		LAR_Y,
		LAR_Z,

		LF_X,
		LF_Y,
		LF_Z,
		LFR_X,
		LFR_Y,
		LFR_Z,

		LV_X,
		LV_Y,
		LV_Z,
		LVR_X,
		LVR_Y,
		LVR_Z,
	};

	InputManager();
	~InputManager();

	void update();
	void refreshBuffer();

	void keyPress(WPARAM keycode);
	void keyUp(WPARAM keycode);

	void mouseButtonDown(UINT buttonId);
	void mouseButtonUp(UINT buttonId);
	void mouseMove(short x, short y);
	void mouseWheel(short delta, short x, short y);

	bool checkKeyboard(UINT keycode, BUTTON_STATE state);
	bool checkGamePad(UINT padId, UINT btnId, BUTTON_STATE state);
	bool checkMouseButton(UINT btnId, BUTTON_STATE state);
	
	XMINT2 getMousePosition();
	XMINT2 getWheel();
	INT getAnalogValue(UINT padId, int analogId);

	void setInputEnable(bool flg);
#if DIRECT_INPUT_ACTIVE
	//DirectInput初期化
	HRESULT initDirectInput(HINSTANCE hInst);
	//DirectInputの終了
	void releaseDirectInput();

	//POVチェック
	bool checkDirectionButton(UINT padId, UINT povId, X_BUTTONS btnId, BUTTON_STATE state);
	//アナログチェック
	LONG getDiAnalogValue(UINT padId, int analogId);
#endif

protected:

#if DIRECT_INPUT_ACTIVE
	enum class DI_POV
	{
		DPOV_UP = 0x01,
		DPOV_RIGHT = 0x02,
		DPOV_DOWN = 0x04,
		DPOV_LEFT = 0x08,
	};
	BYTE m_diDirection[MAX_DI_PADS][4];	//POVは最大４つ。
	BYTE m_diButtonState[MAX_DI_PADS][MAX_DI_BUTTONS];
	LONG m_diAnalogState[MAX_DI_PADS][MAX_DI_ANALOGIN];

	void releaseDIGameController(int conId);
	void releaseAllDIControllers();
#endif

	BYTE m_keyState[MAX_KEYS];
	BYTE m_mouseState[MAX_MOUSE_BUTTONS];
	BYTE m_buttonState[MAX_PADS][MAX_PAD_BUTTONS];
	LONG m_analogState[MAX_PADS][MAX_PAD_ANALOGIN];

	bool m_padActive[MAX_PADS];

	XMINT2 m_mousePos = { 0, 0 };
	XMINT2 m_wheelPos = { 0, 0 };

	void setXButton(UINT padId, UINT btnId, bool push);
	bool checkState(UINT flg, BUTTON_STATE state);
};