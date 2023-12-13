#include "GameOverUIRender.h"

#include <MyAccessHub.h>
#include <D3D12Helper.h>
#include <stdio.h>

void GameOverUIRender::initAction()
{
	UIRenderBase::initAction();
}

bool GameOverUIRender::frameAction()
{
	MyGameEngine* engine = MyAccessHub::getMyGameEngine();
	GraphicsPipeLineObjectBase* pipeLine = engine->GetPipelineManager()->GetPipeLineObject(L"Sprite");

	float x = -480.0f + 30.0f;	//中心0,0 w960 h540
	float y = 270.0f - 30.0f;

	int count = 0;
	count = MakeSpriteString(count, -215, -180, 24, 24, retryText);
	count = MakeSpriteString(count, -170, -210, 24, 24, titleText);


	//使ったSpriteCharacterだけをパイプラインに登録
	for (int i = 0; i < count; i++)
	{
		pipeLine->AddRenerObject(m_sprites[i].get());
	}

	return true;
}

void GameOverUIRender::finishAction()
{
	UIRenderBase::finishAction();
}