﻿#include "GameOverUIRender.h"

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

	//中心0,0 w960 h540
	float x = -480.0f + 30.0f;	//-450.0f
	float y = 270.0f - 30.0f;	// 240.0f

	int count = 0;
	count = MakeSpriteString(count, x + 235, y - 420, 24, 24, retryText);
	count = MakeSpriteString(count, x + 280, y - 450, 24, 24, titleText);


	//使ったSpriteCharacterだけをパイプラインに登録
	for (int i = 0; i < count; i++)
	{
		pipeLine->AddRenerObject(m_sprites[i].get());
	}

	//======シーン遷移処理
	GamePrograming3Scene* scene = static_cast<GamePrograming3Scene*>(engine->GetSceneController());
	KeyBindComponent* keyBind = static_cast<KeyBindComponent*>(scene->getKeyComponent());

	//Title遷移
	if (keyBind->getCurrentInputState(InputManager::BUTTON_STATE::BUTTON_DOWN, KeyBindComponent::BUTTON_IDS::BTN_JUMP))
		scene->OrderNextScene((UINT)GAME_SCENES::TITLE);

	//Retry(IN_GAME)遷移
	if (keyBind->getCurrentInputState(InputManager::BUTTON_STATE::BUTTON_DOWN, KeyBindComponent::BUTTON_IDS::BUTTON_R))
		scene->OrderNextScene((UINT)GAME_SCENES::IN_GAME);
	//======シーン遷移処理 End

	return true;
}

void GameOverUIRender::finishAction()
{
	UIRenderBase::finishAction();
}