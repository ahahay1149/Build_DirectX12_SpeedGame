#include "TitleUIRender.h"

#include <MyAccessHub.h>
#include <D3D12Helper.h>
#include <stdio.h>

#include "GamePrograming3Scene.h"
#include "GamePrograming3Enum.h"
#include "HeartItemComponent.h"

void TitleUIRender::initAction()
{
	UIRenderBase::initAction();
}

bool TitleUIRender::frameAction()
{
	MyGameEngine* engine = MyAccessHub::getMyGameEngine();
	GamePrograming3Scene* scene = GamePrograming3Scene::getScene();
	GraphicsPipeLineObjectBase* pipeLine = engine->GetPipelineManager()->GetPipeLineObject(L"Sprite");

	float x = -480.0f + 30.0f;	//中心0,0 w960 h540
	float y = 270.0f - 30.0f;

	int count = 0;
	count = MakeSpriteString(count, -160, -180, 24, 24, uiText);

	//使ったSpriteCharacterだけをパイプラインに登録
	for (int i = 0; i < count; i++)
	{
		pipeLine->AddRenerObject(m_sprites[i].get());
	}

	return true;
}

void TitleUIRender::finishAction()
{
	UIRenderBase::finishAction();
}