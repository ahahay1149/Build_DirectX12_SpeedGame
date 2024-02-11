﻿#include <MyAccessHub.h>
#include "KeyBindComponent.h"

KeyBindComponent::InputConfigData::InputConfigData()
{
	m_analogMap[ANALOG_IDS::MOVE_H] = static_cast<UINT32>(InputManager::X_ANALOGS::L_X);		//L
	m_analogMap[ANALOG_IDS::MOVE_V] = static_cast<UINT32>(InputManager::X_ANALOGS::L_Y);		//L
	m_analogMap[ANALOG_IDS::CAMERA_V] = static_cast<UINT32>(InputManager::X_ANALOGS::R_Y);		//R
	m_analogMap[ANALOG_IDS::CAMERA_H] = static_cast<UINT32>(InputManager::X_ANALOGS::R_X);		//R
	m_analogMap[ANALOG_IDS::WHEEL] = static_cast<UINT32>(InputManager::X_ANALOGS::R_TRIGGER);	//THUMB

	m_buttonMap[BUTTON_IDS::BTN_ATTACK] = static_cast<UINT32>(InputManager::X_BUTTONS::BTN_B);
	m_buttonMap[BUTTON_IDS::BTN_JUMP] = static_cast<UINT32>(InputManager::X_BUTTONS::BTN_A);
	m_buttonMap[BUTTON_IDS::BTN_OK] = static_cast<UINT32>(InputManager::X_BUTTONS::BTN_A);
	m_buttonMap[BUTTON_IDS::BTN_CANCEL] = static_cast<UINT32>(InputManager::X_BUTTONS::BTN_B);
}

KeyBindComponent::InputConfigData::~InputConfigData()
{
	m_analogMap.clear();
	m_buttonMap.clear();
}

bool KeyBindComponent::frameAction()
{
	m_mousePrePos = m_mousePos;
	m_mousePos = m_inputMng->getMousePosition();

	return true;
} 

void KeyBindComponent::finishAction()
{
}

bool KeyBindComponent::getCurrentInputState(InputManager::BUTTON_STATE state, BUTTON_IDS buttonId)
{
	return getInputState(m_currentInputType, state, buttonId);
}

LONG KeyBindComponent::getCurrentAnalogValue(ANALOG_IDS analogId)
{
	return getAnalogValue(m_currentInputType, analogId);
}

bool KeyBindComponent::getInputState(INPUT_TYPES inputDev, InputManager::BUTTON_STATE state, BUTTON_IDS buttonId)
{
	InputConfigData* conf = m_configData[inputDev].get();

	switch (buttonId)
	{
	case BUTTON_IDS::MOUSE_L:
		return m_inputMng->checkMouseButton(0, state);
		break;
	case BUTTON_IDS::MOUSE_R:
		return m_inputMng->checkMouseButton(1, state);
		break;
	case BUTTON_IDS::MOUSE_C:
		return m_inputMng->checkMouseButton(2, state);
		break;

	default:
		{
			switch (inputDev)
			{
			case INPUT_TYPES::KEYBOARD:
				return m_inputMng->checkKeyboard(conf->m_buttonMap[buttonId], state);

			case INPUT_TYPES::XINPUT_0:
			case INPUT_TYPES::XINPUT_1:
			case INPUT_TYPES::XINPUT_2:
			case INPUT_TYPES::XINPUT_3:
			{
				//XINPUT
				UINT x_id = static_cast<UINT>(inputDev) - static_cast<UINT>(INPUT_TYPES::XINPUT_0);
				return m_inputMng->checkGamePad(x_id, conf->m_buttonMap[buttonId], state);
			}
			break;

			default:
			{
				//DIRECTINPUT
				UINT d_id = static_cast<UINT>(inputDev) - static_cast<UINT>(INPUT_TYPES::XINPUT_0);
				return m_inputMng->checkGamePad(d_id, conf->m_buttonMap[buttonId], state);
			}
			break;
			}
		}
		break;
	}

	return false;
}

LONG KeyBindComponent::getAnalogValue(INPUT_TYPES inputDev, ANALOG_IDS analogId)
{
	LONG res = 0;
	switch (inputDev)
	{
	case INPUT_TYPES::KEYBOARD:
		{
			switch (analogId)
			{
			case ANALOG_IDS::CAMERA_H:
				res = m_mousePos.x - m_mousePrePos.x;
				break;

			case ANALOG_IDS::CAMERA_V:
				res = m_mousePos.y - m_mousePrePos.y;
				break;

			case ANALOG_IDS::WHEEL:
				res = m_inputMng->getWheel().y;
				break;

			case ANALOG_IDS::WHEEL_H:
				res = m_inputMng->getWheel().x;
				break;

			default:	//キーボードの場合これ以外のアナログ値はない
				break;
			}
		}
		break;

	case INPUT_TYPES::XINPUT_0:
	case INPUT_TYPES::XINPUT_1:
	case INPUT_TYPES::XINPUT_2:
	case INPUT_TYPES::XINPUT_3:
		{
			InputConfigData* conf = m_configData[inputDev].get();

			//XINPUT
			UINT x_id = static_cast<UINT>(inputDev) - static_cast<UINT>(INPUT_TYPES::XINPUT_0);

			res = m_inputMng->getAnalogValue(x_id, conf->m_analogMap[analogId]);
		}
		break;

	default:
		{
			InputConfigData* conf = m_configData[inputDev].get();

			//DIRECTINPUT
			UINT d_id = static_cast<UINT>(inputDev) - static_cast<UINT>(INPUT_TYPES::DINPUT_0);

			res = m_inputMng->getDiAnalogValue(d_id, conf->m_analogMap[analogId]);
		}
		break;
	}

	return res;
}

void KeyBindComponent::initAction()
{
	m_inputMng = MyAccessHub::getMyGameEngine()->GetInputManager();
	m_currentInputType = INPUT_TYPES::KEYBOARD;

	InputConfigData* conf;

	m_configData[INPUT_TYPES::KEYBOARD] = make_unique<InputConfigData>();

	m_configData[INPUT_TYPES::KEYBOARD]->m_buttonMap[BUTTON_IDS::MOVE_FORWARD] = 'W';
	m_configData[INPUT_TYPES::KEYBOARD]->m_buttonMap[BUTTON_IDS::MOVE_LEFT] = 'A';
	m_configData[INPUT_TYPES::KEYBOARD]->m_buttonMap[BUTTON_IDS::MOVE_BACK] = 'S';
	m_configData[INPUT_TYPES::KEYBOARD]->m_buttonMap[BUTTON_IDS::MOVE_RIGHT] = 'D';
	m_configData[INPUT_TYPES::KEYBOARD]->m_buttonMap[BUTTON_IDS::BTN_JUMP] = VK_SPACE;

	m_configData[INPUT_TYPES::KEYBOARD]->m_buttonMap[BUTTON_IDS::BUTTON_Q] = 'Q';
	m_configData[INPUT_TYPES::KEYBOARD]->m_buttonMap[BUTTON_IDS::BUTTON_E] = 'E';
	m_configData[INPUT_TYPES::KEYBOARD]->m_buttonMap[BUTTON_IDS::BUTTON_R] = 'R';

}

