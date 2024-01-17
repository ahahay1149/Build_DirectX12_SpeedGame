#include "ImguiManager.h"

#include "GamePrograming3Scene.h"
#include "KeyBindComponent.h"
#include "GamePrograming3Enum.h"

void ImguiManager::initAction()
{

}

bool ImguiManager::frameAction()
{
	auto scene	= static_cast<GamePrograming3Scene*>(MyAccessHub::getMyGameEngine()->GetSceneController());
	auto key	= static_cast<KeyBindComponent*>(scene->getKeyComponent());

	if (!key->getWindowBool())
		return true;

	m_scene = scene->getCurrentScene();

	switch (m_scene)
	{
		case static_cast<UINT>(GAME_SCENES::IN_GAME):
		{
			ImGui::Begin("Window");
			ImGui::Checkbox("Imgui:Shader", &shaderCheck);
			if (shaderCheck)
				imguiShaderDebug();
			ImGui::End();

			ImGui::Begin("Window");
			ImGui::Checkbox("Imgui:SetActive", &setActiveCheck);
			if (setActiveCheck)
				imguiSetActiveDebug();
			ImGui::End();
		}
		break;
	}

	return true;
}

void ImguiManager::finishAction()
{

}

void ImguiManager::setImguiObject(std::string id, ImguiProcessing* imgui, UINT32 flag)
{
	m_imguiComponents.insert_or_assign(id, imgui);
	imgui->setId(id);
	imgui->setDebugFlag(flag);
	imgui->imguiInit();

	GameComponent* gameObj = dynamic_cast<GameComponent*>(imgui);
	if (gameObj == nullptr)
		assert(false && "ImguiManager::chData input failed");

	imgui->setComData(gameObj);
}

void ImguiManager::imguiSetActiveDebug()
{
	ImGui::Begin("SetActive");
	ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_::ImGuiCond_FirstUseEver);

	ImGui::SeparatorText("HeartItem");
	for (auto& it : m_imguiComponents)
	{
		if (it.second->getDebugFlag() & DEBUG_FLAG::Component_HeartItem)
		{
			GameComponent* gameCom = it.second->getComData();
			bool heartActive = gameCom->isActive();
			if (ImGui::Checkbox((it.first).c_str(), &heartActive))
			{
				gameCom->setActive(!gameCom->isActive());
			}
		}
	}

	ImGui::End();
}

void ImguiManager::imguiShaderDebug()
{
	ImGui::Begin("Shader");
	ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_::ImGuiCond_FirstUseEver);

	if (ImGui::Combo("StageShader", &stageShader, items, IM_ARRAYSIZE(items)))
	{
		for (auto& it : m_imguiComponents)
		{
			if (it.second->getDebugFlag() & DEBUG_FLAG::Shader_Stage)
			{
				FBXCharacterData* chdata = static_cast<FBXCharacterData*>(it.second->getComData()->getGameObject()->getCharacterData());
				switch (stageShader)
				{
				case 0:
					chdata->SetGraphicsPipeLine(L"StaticLambert");		//FBX+Lambert
					break;
				case 1:
					chdata->SetGraphicsPipeLine(L"StaticPhong");		//FBX+Phong
					break;
				case 2:
					chdata->SetGraphicsPipeLine(L"StaticBlinn");		//FBX+BlinnPhong
					break;
				case 3:
					chdata->SetGraphicsPipeLine(L"StaticToon");			//FBX+Toon
					break;
				}
			}
		}
	}

	if (ImGui::Combo("PlayerShader", &playerShader, items, IM_ARRAYSIZE(items)))
	{
		for (auto& it : m_imguiComponents)
		{
			if (it.second->getDebugFlag() & DEBUG_FLAG::Shader_Player)
			{
				FBXCharacterData* chdata = static_cast<FBXCharacterData*>(it.second->getComData()->getGameObject()->getCharacterData());
				switch (playerShader)
				{
				case 0:
					chdata->SetGraphicsPipeLine(L"SkeltalLambert");		//スキンアニメ有りFBX+Lambert
					break;
				case 1:
					chdata->SetGraphicsPipeLine(L"SkeltalPhong");		//スキンアニメ有りFBX+Phong
					break;
				case 2:
					chdata->SetGraphicsPipeLine(L"SkeltalBlinn");		//スキンアニメ有りFBX+BlinnPhong
					break;
				case 3:
					chdata->SetGraphicsPipeLine(L"SkeltalToon");		//スキンアニメ有りFBX+Toon
					break;
				}
			}
		}
	}

	ImGui::End();
}
