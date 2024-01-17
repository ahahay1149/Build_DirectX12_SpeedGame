#pragma once
#include "GameObject.h"
#include "ImguiProcessing.h"
#include "GamePrograming3Enum.h"

//ImGui
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#include <map>

#include "MapCustomCompare.h"

class ImguiManager : public GameComponent
{
private:
	std::map<std::string, ImguiProcessing*, ComponentCustomCompare> m_imguiComponents;

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

	void setImguiObject(std::string id, ImguiProcessing* imgui, UINT32 flag);
};