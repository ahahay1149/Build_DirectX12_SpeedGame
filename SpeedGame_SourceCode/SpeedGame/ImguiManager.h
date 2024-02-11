#pragma once
#include "GameObject.h"
#include "ImguiComponent.h"

//定義系
#include "GamePrograming3Enum.h"
#include "MapCustomCompare.h"

//ImGui
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#include <map>

class ImguiManager : public GameComponent
{
private:
	std::map<std::string, ImguiComponent*, ComponentCustomCompare> m_imguiComponents;

	bool windowBool = false;

	const char* items[4] = { "Lambert","Phong","Blinn Phong", "Toon" };
	int stageShader = -1;
	int playerShader = -1;

	bool shaderCheck = false;
	bool setActiveCheck = false;

	UINT m_scene;

	void imguiSetActiveDebug();
	void imguiShaderDebug();

public:

	// GameComponent を介して継承されました
	void initAction() override;
	bool frameAction() override;
	void finishAction() override;

	void setImguiObject(std::string id, ImguiComponent* imgui, UINT32 flag);
	void clearImguiObject(UINT scene);
};