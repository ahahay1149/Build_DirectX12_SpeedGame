#include "HeartItemComponent.h"
#include "FBXCharacterData.h"	//FBXCharacterDataを使うので

#include "GameAccessHub.h"
#include "GamePrograming3Enum.h"

#include "UnityChanPlayer.h"

void HeartItemComponent::initAction()
{
	//FBXCharacterDataは基底クラスのGameObjectにセットする
	FBXCharacterData* chdata = static_cast<FBXCharacterData*>(getGameObject()->getCharacterData());
	//chdata->SetGraphicsPipeLine(L"StaticFBX");
	//chdata->SetGraphicsPipeLine(L"StaticLambert");	//Lambert
	//chdata->SetGraphicsPipeLine(L"StaticPhong");		//Phong
	//chdata->SetGraphicsPipeLine(L"StaticBlinn");		//Blinn Phong
	chdata->SetGraphicsPipeLine(L"StaticToon");			//Toon

	chdata->setScale(0.01f, 0.01f, 0.01f);	//元モデルがかなり大きい（というかスカイドームとかとあってない）ので縮小

	XMFLOAT3 scl = chdata->getScale();
	XMFLOAT3 min = chdata->GetMainFbx()->GetFbxMin();
	XMFLOAT3 max = chdata->GetMainFbx()->GetFbxMax();

	m_centerY = (max.y - min.y) * scl.y * 0.5f;	//0.01 scale

	m_itemHit.setRadius(m_centerY * 0.8f);	//かなり適当な半径設定
	m_itemHit.setAttackType((UINT)HIT_ORDER::HIT_ITEM, 0);
	sta = 0;
}

bool HeartItemComponent::frameAction()
{
	switch (sta)
	{
	case 0:
	{
		FBXCharacterData* chData = static_cast<FBXCharacterData*>(getGameObject()->getCharacterData());

		chData->GetPipeline()->AddRenerObject(chData);

		XMFLOAT3 pos = chData->getPosition();

		m_itemHit.setCenter(pos.x, pos.y + m_centerY, pos.z);

		MyAccessHub::getMyGameEngine()->GetHitManager()->setHitArea(this, &m_itemHit);

		//ここでハートの位置を移動
		srand((unsigned int)time(nullptr));
		rotation.y += 0.1f * (rand() % 5 + 1);
		if (rotation.y > 360.0f)
			rotation.y = 0.0f;
		chData->setRotation(rotation.x, rotation.y, rotation.z);
	}
		break;

	default:
		//取得後
		break;
	}


	return true;
}

void HeartItemComponent::finishAction()
{
}

void HeartItemComponent::hitReaction(GameObject* targetGo, HitAreaBase* hit)
{
	sta = 1;
	UnityChanPlayer* unityChan = GameAccessHub::getUnityChan();
	unityChan->plusSpeedCount(m_playerSpeed);
}

void HeartItemComponent::setPlayerSpeed(float playerSpeed)
{
	m_playerSpeed = playerSpeed;
}

float HeartItemComponent::getPlayerSpeed()
{
	return m_playerSpeed;
}
