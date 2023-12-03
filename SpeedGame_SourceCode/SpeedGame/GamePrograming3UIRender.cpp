#include "GamePrograming3UIRender.h"

#include <MyAccessHub.h>
#include <D3D12Helper.h>

#include "GamePrograming3Scene.h"
#include "GamePrograming3Enum.h"
#include "HeartItemComponent.h"
#include "UnityChanPlayer.h"

void GamePrograming3UIRender::initAction()
{
	UIRenderBase::initAction();
}

bool GamePrograming3UIRender::frameAction()
{
	MyGameEngine* engine = MyAccessHub::getMyGameEngine();
	GraphicsPipeLineObjectBase* pipeLine = engine->GetPipelineManager()->GetPipeLineObject(L"Sprite");

	GamePrograming3Scene* scene = GamePrograming3Scene::getScene();

	int count = 0;
	float x = -480.0f + 30.0f;	//中心0,0 w960 h540
	float y = 270.0f - 30.0f;

	//開始前のカウントダウンのUI制御
	startCount -= 0.01666f;
	if (startCount > 3.0f)
	{
		sprintf_s(countText, "  %d", 3);
	}
	else if (startCount > 2.0f)
	{
		sprintf_s(countText, "  %d", 2);
	}
	else if (startCount > 1.0f)
	{
		sprintf_s(countText, "  %d", 1);
	}
	else if (startCount > 0.0f)
	{
		sprintf_s(countText, "Start!");
	}
	else
	{
		sprintf_s(countText, "");

		//ゲーム中のカウントダウンUI制御
		timerCount -= 0.01666f;
		sprintf_s(timerText, "Time %.3f", timerCount);
		//共通のタイマーシステムまだ作ってないので、UIの遷移もここでやってしまう
		if (timerCount < 0)
		{
			engine->GetSceneController()->OrderNextScene((UINT)GAME_SCENES::GAME_OVER);
		}
	}

	//ハートアイテムのUI制御。取得数カウント
	sprintf_s(itemText, "Item %d / 5", heartItemCount);

	count = MakeSpriteString(count, x, y, 24, 24, itemText);
	count = MakeSpriteString(count, x, y - 60, 24, 24, timerText);

	count = MakeSpriteString(count, -100, 0, 48, 48, countText);

	if (heartItemCount >= 5)
	{
		scene->setClearCount(timerCount);
		engine->GetSceneController()->OrderNextScene((UINT)GAME_SCENES::GAME_CLEAR);
	}

	//使ったSpriteCharacterだけをパイプラインに登録
	for (int i = 0; i < count; i++)
	{
		pipeLine->AddRenerObject(m_sprites[i].get());
	}

	return true;
}

void GamePrograming3UIRender::finishAction()
{
	UIRenderBase::finishAction();
}

void GamePrograming3UIRender::plusHeartItemCount()
{
	heartItemCount++;
}