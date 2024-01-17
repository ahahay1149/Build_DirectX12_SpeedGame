#include "ImguiProcessing.h"

#include "GamePrograming3Scene.h"
#include "KeyBindComponent.h"

void ImguiProcessing::imgui()
{
}

void ImguiProcessing::imguiInit()
{
}

bool ImguiProcessing::imguiSetting()
{
	auto scene	= static_cast<GamePrograming3Scene*>(MyAccessHub::getMyGameEngine()->GetSceneController());
	auto key	= static_cast<KeyBindComponent*>(scene->getKeyComponent());

	return key->getWindowBool();
}