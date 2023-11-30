#include <MyAccessHub.h>

#include "SkyDomeComponent.h"
#include "GamePrograming3Scene.h"

void SkyDomeComponent::initAction()
{
	CharacterData* chdata = getGameObject()->getCharacterData();
	chdata->setScale(10.0f, 10.0f, 10.0f);	//モデルが小さいのでXYZともに10倍

	//01: パイプライン設定
	chdata->SetGraphicsPipeLine(L"StaticFBX");	//アニメなしFBXモード
	//01: ここまで

}

bool SkyDomeComponent::frameAction()
{
	CharacterData* myData = getGameObject()->getCharacterData();

	//スカイドームの中心は常にキャラクタの中心に移動する
	if (centerCharacter != nullptr)
	{
		XMFLOAT3 charaPos = centerCharacter->getPosition();

		myData->setPosition(charaPos.x, charaPos.y, charaPos.z);
	}

	//02: PipeLineに登録
	myData->GetPipeline()->AddRenerObject(myData);
	//02; ここまで
	return true;
}

void SkyDomeComponent::finishAction()
{
}

void SkyDomeComponent::setCenterCharacter(CharacterData* target)
{
	centerCharacter = target;
}
