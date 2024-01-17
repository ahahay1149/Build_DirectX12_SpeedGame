#include "HeartItemComponent.h"
#include "FBXCharacterData.h"	//FBXCharacterDataを使うので

#include "GameAccessHub.h"
#include "GamePrograming3Enum.h"

void HeartItemComponent::initAction()
{
	//FBXCharacterDataは基底クラスのGameObjectにセットする
	FBXCharacterData* chdata = static_cast<FBXCharacterData*>(getGameObject()->getCharacterData());
	//chdata->SetGraphicsPipeLine(L"StaticFBX");
	//chdata->SetGraphicsPipeLine(L"StaticLambert");		//Lambert
	chdata->SetGraphicsPipeLine(L"StaticPhong");			//Phong
	//chdata->SetGraphicsPipeLine(L"StaticBlinn");			//Blinn Phong
	//chdata->SetGraphicsPipeLine(L"StaticToon");			//Toon

	chdata->setScale(0.01f, 0.01f, 0.01f);	//元モデルがかなり大きい（というかスカイドームとかとあってない）ので縮小

	XMFLOAT3 scl = chdata->getScale();
	XMFLOAT3 min = chdata->GetMainFbx()->GetFbxMin();
	XMFLOAT3 max = chdata->GetMainFbx()->GetFbxMax();

	m_centerY = (max.y - min.y) * scl.y * 0.5f;	//0.01 scale

	m_itemHit.setRadius(m_centerY * 0.8f);	//かなり適当な半径設定
	m_itemHit.setAttackType((UINT)HIT_ORDER::HIT_ITEM, 0);
}

bool HeartItemComponent::frameAction()
{
	bool activeCheck = isActive();
	if (activeCheck == true)
	{
		FBXCharacterData* chData = static_cast<FBXCharacterData*>(getGameObject()->getCharacterData());

		chData->GetPipeline()->AddRenerObject(chData);

		m_pos = chData->getPosition();

		m_itemHit.setCenter(m_pos.x, m_pos.y + m_centerY, m_pos.z);

		MyAccessHub::getMyGameEngine()->GetHitManager()->setHitArea(this, &m_itemHit);

		//ここでハートの向きを動かす
		srand((unsigned int)time(nullptr));
		m_rotation.y += 0.1f * (rand() % 5 + 1);
		if (m_rotation.y > 360.0f)
			m_rotation.y = 0.0f;
		chData->setRotation(m_rotation.x, m_rotation.y, m_rotation.z);
	}

	//ImGui
	imgui();

	return true;
}

void HeartItemComponent::finishAction()
{
}

void HeartItemComponent::hitReaction(GameObject* targetGo, HitAreaBase* hit)
{
	setActive(false);
	GameAccessHub::getUnityChan()->setSpeedCount(m_playerSpeed);
	GameAccessHub::getGameManager()->setPlusHeartItemCount(m_heartPoint);

}

void HeartItemComponent::imgui()
{
	if (!ImguiProcessing::imguiSetting())
		return;

	FBXCharacterData* chdata = static_cast<FBXCharacterData*>(getGameObject()->getCharacterData());

	ImGui::Begin("Window");
	ImGui::BeginChild(ImGui::GetID((void*)0), ImVec2(250, 100), ImGuiWindowFlags_NoTitleBar);
	ImGui::Checkbox((id).c_str(), &check);
	ImGui::EndChild();
	ImGui::End();

	if (check == true)
	{
		ImGui::Begin("HeartItem");
		ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_::ImGuiCond_FirstUseEver);

		ImGui::PushID(id.c_str());
		ImGui::SeparatorText((id).c_str());

		if (ImGui::SliderFloat3("Position", &m_pos.x, -30.0f, 30.0f))
			chdata->setPosition(m_pos.x, m_pos.y, m_pos.z);

		ImGui::SliderFloat("Rotate", &m_rotation.y, 0.0f, 360.0f);

		if (ImGui::Button("ResetPos"))
			chdata->setPosition(m_initPos.x, m_initPos.y, m_initPos.z);

		ImGui::PopID();

		ImGui::End();
	}
}

void HeartItemComponent::imguiInit()
{
	FBXCharacterData* chdata = static_cast<FBXCharacterData*>(getGameObject()->getCharacterData());
	m_initPos = chdata->getPosition();
}
