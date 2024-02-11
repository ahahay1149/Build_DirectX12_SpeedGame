#include "HeartItemComponent.h"
#include "FBXCharacterData.h"	//FBXCharacterDataを使うので

#include "GameAccessHub.h"
#include "GamePrograming3Enum.h"
#include "GameAppEnum.h"		//HeartType

void HeartItemComponent::initAction()
{
	//FBXCharacterDataは基底クラスのGameObjectにセットする
	FBXCharacterData* chdata = static_cast<FBXCharacterData*>(getGameObject()->getCharacterData());
	//chdata->SetGraphicsPipeLine(L"StaticFBX");
	//chdata->SetGraphicsPipeLine(L"StaticLambert");		//Lambert
	//chdata->SetGraphicsPipeLine(L"StaticPhong");			//Phong
	//chdata->SetGraphicsPipeLine(L"StaticBlinn");			//Blinn Phong
	chdata->SetGraphicsPipeLine(L"StaticToon");				//Toon

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
		m_rotation.y += 0.1f * 5;
		if (m_rotation.y > 360.0f)
			m_rotation.y = 0.0f;
		chData->setRotation(m_rotation.x, m_rotation.y, m_rotation.z);
	}

	return true;
}

void HeartItemComponent::finishAction()
{
}

void HeartItemComponent::hitReaction(GameObject* targetGo, HitAreaBase* hit)
{
	setActive(false);
	GameManager* gameManager = GameAccessHub::getGameManager();
	gameManager->setPlusHeartItemCount(m_heartPoint);

	//各ハートごとのポイント処理
	switch (m_heartPoint)
	{
	case static_cast<int>(Heart::TYPE::Gold):
	case static_cast<int>(Heart::TYPE::Red):
		gameManager->varSetPlayerSpeed(m_heartSpeed);
		break;
	case static_cast<int>(Heart::TYPE::Blue):
		gameManager->varSetPlayerSpeed(static_cast<int>(Heart::TYPE::Blue));
		break;
	}

	//各ハートごとの効果音
	switch (m_heartPoint)
	{
	case static_cast<int>(Heart::TYPE::Gold):
		MyAccessHub::getMyGameEngine()->GetSoundManager()->play(8);
		break;
	case static_cast<int>(Heart::TYPE::Red):
		MyAccessHub::getMyGameEngine()->GetSoundManager()->play(9);
		break;
	case static_cast<int>(Heart::TYPE::Blue):
		MyAccessHub::getMyGameEngine()->GetSoundManager()->play(10);
		break;
	}
}

void HeartItemComponent::imgui()
{
	FBXCharacterData* chdata = static_cast<FBXCharacterData*>(getGameObject()->getCharacterData());

	ImGui::Begin("Window");
	ImGui::BeginChild(ImGui::GetID((void*)0), ImVec2(250, 125), ImGuiWindowFlags_NoTitleBar);
	ImGui::Checkbox((id).c_str(), &check);
	ImGui::EndChild();
	ImGui::End();

	if (check == true)
	{
		ImGui::Begin("HeartItem");
		ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_::ImGuiCond_FirstUseEver);

		ImGui::PushID(id.c_str());
		ImGui::SeparatorText((id).c_str());

		//各軸に対するPosition移動処理
		if (ImGui::SliderFloat("PositionX", &m_pos.x, m_initPos.x - 30.0f, m_initPos.x + 30.0f))
			chdata->setPosition(m_pos.x, m_pos.y, m_pos.z);
		if (ImGui::SliderFloat("PositionY", &m_pos.y, m_initPos.y - 30.0f, m_initPos.y + 30.0f))
			chdata->setPosition(m_pos.x, m_pos.y, m_pos.z);
		if (ImGui::SliderFloat("PositionZ", &m_pos.z, m_initPos.z - 30.0f, m_initPos.z + 30.0f))
			chdata->setPosition(m_pos.x, m_pos.y, m_pos.z);

		ImGui::SliderFloat("Rotate", &m_rotation.y, 0.0f, 360.0f);

		if (ImGui::Button("SavePos"))
			m_savePos = m_pos;

		ImGui::SameLine();
		if (ImGui::Button("LoadPos"))
			chdata->setPosition(m_savePos.x, m_savePos.y, m_savePos.z);

		ImGui::SameLine();
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
