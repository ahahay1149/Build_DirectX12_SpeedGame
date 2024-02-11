#include "ResultUIRender.h"

#include <MyAccessHub.h>
#include <D3D12Helper.h>
#include <stdio.h>

#include "GamePrograming3Scene.h"
#include "GameAccessHub.h"

#include "ScoreDataContainer.h"

void ResultUIRender::initAction()
{
	UIRenderBase::initAction();

	MyGameEngine* engine = MyAccessHub::getMyGameEngine();
	GamePrograming3Scene* scene = static_cast<GamePrograming3Scene*>(engine->GetSceneController());

	//前のシーン情報を取得
	GAME_SCENES preScene = static_cast<GAME_SCENES>(scene->getPreviousScene());

	//前のシーンがIN_GAME03のみ表示を変更
	if (preScene != GAME_SCENES::IN_GAME03)
	{
		strcpy_s(nextText, "NextStage/ R Key");
		setX = 290;
	}
	else if (preScene == GAME_SCENES::IN_GAME03)
	{
		strcpy_s(nextText, "Restart/ R key");
		setX = 330;
	}

	using sd = ScoreDataContainer;

	//各ステージのタイムをテキスト形式に変形
	//Stage01
	sprintf_s(stage[0].clearTime,	"Stage01 Time [%.3f]",	sd::stage[0].m_clearTime);
	sprintf_s(stage[0].bestTime,	"BestTime <%.3f>",		sd::stage[0].m_bestTime);
	//Stage02
	sprintf_s(stage[1].clearTime,	"Stage02 Time [%.3f]",	sd::stage[1].m_clearTime);
	sprintf_s(stage[1].bestTime,	"BestTime <%.3f>",		sd::stage[1].m_bestTime);
	//Stage03
	sprintf_s(stage[2].clearTime,	"Stage03 Time [%.3f]",	sd::stage[2].m_clearTime);
	sprintf_s(stage[2].bestTime,	"BestTime <%.3f>",		sd::stage[2].m_bestTime);
	//Result
	sprintf_s(stage[3].clearTime,	" Result Time [%.3f]",	sd::stage[3].m_clearTime);
	sprintf_s(stage[3].bestTime,	"BestTime <%.3f>",		sd::stage[3].m_bestTime);
}

bool ResultUIRender::frameAction()
{
	MyGameEngine* engine = MyAccessHub::getMyGameEngine();
	GraphicsPipeLineObjectBase* pipeLine = engine->GetPipelineManager()->GetPipeLineObject(L"Sprite");

	//中心0,0 w960 h540
	float x = -480.0f + 30.0f;	//-450.0f
	float y = 270.0f - 30.0f;	// 240.0f

	int count = 0;

	//各ステージのタイムを表示
	//Stage01
	count = MakeSpriteString(count, x + 140, y - 320, 16, 16, stage[0].clearTime);
	count = MakeSpriteString(count, x + 490, y - 320, 16, 16, stage[0].bestTime);
	//Stage02
	count = MakeSpriteString(count, x + 140, y - 345, 16, 16, stage[1].clearTime);
	count = MakeSpriteString(count, x + 490, y - 345, 16, 16, stage[1].bestTime);
	//Stage03
	count = MakeSpriteString(count, x + 140, y - 370, 16, 16, stage[2].clearTime);
	count = MakeSpriteString(count, x + 490, y - 370, 16, 16, stage[2].bestTime);
	//Result
	count = MakeSpriteString(count, x + 65,  y - 400, 20, 20, stage[3].clearTime);
	count = MakeSpriteString(count, x + 490, y - 400, 20, 20, stage[3].bestTime);
	
	count = MakeSpriteString(count, x + setX, y - 450, 18, 18, nextText);
	count = MakeSpriteString(count, x + 360,  y - 480, 18, 18, titleText);

	using sd = ScoreDataContainer;

	//各ステージのタイム更新時の点滅演出
	if (sd::stage[0].m_updateTime == true)
		displayScoreUpdate(count, (int)x + 830, (int)y - 320);
	if (sd::stage[1].m_updateTime == true)
		displayScoreUpdate(count, (int)x + 830, (int)y - 345);
	if (sd::stage[2].m_updateTime == true)
		displayScoreUpdate(count, (int)x + 830, (int)y - 370);
	if (sd::stage[3].m_updateTime == true)
		displayScoreUpdate(count, (int)x + 830, (int)y - 400);

	//使ったSpriteCharacterだけをパイプラインに登録
	for (int i = 0; i < count; i++)
	{
		pipeLine->AddRenerObject(m_sprites[i].get());
	}

	//======シーン遷移処理
	GamePrograming3Scene* scene = static_cast<GamePrograming3Scene*>(engine->GetSceneController());
	KeyBindComponent* keyBind = static_cast<KeyBindComponent*>(scene->getKeyComponent());

	if (keyBind->getCurrentInputState(InputManager::BUTTON_STATE::BUTTON_DOWN, KeyBindComponent::BUTTON_IDS::BTN_JUMP))
		scene->OrderNextScene((UINT)GAME_SCENES::TITLE);

	//NextStage分岐遷移
	if (keyBind->getCurrentInputState(InputManager::BUTTON_STATE::BUTTON_DOWN, KeyBindComponent::BUTTON_IDS::BUTTON_R))
	{
		switch (static_cast<GAME_SCENES>(scene->getPreviousScene()))
		{
		case GAME_SCENES::IN_GAME:
			scene->OrderNextScene((UINT)GAME_SCENES::IN_GAME02);
			break;

		case GAME_SCENES::IN_GAME02:
			scene->OrderNextScene((UINT)GAME_SCENES::IN_GAME03);
			break;

		case GAME_SCENES::IN_GAME03:
			scene->OrderNextScene((UINT)GAME_SCENES::IN_GAME);
			break;

		//どれにも属していない場合(デバッグ等)
		default:
			scene->OrderNextScene((UINT)GAME_SCENES::TITLE);
		}
	}
	//======シーン遷移処理 End

	return true;
}

void ResultUIRender::finishAction()
{
	UIRenderBase::finishAction();
}

void ResultUIRender::displayScoreUpdate(int& count, int x, int y)
{
	scoreUpdateTimer += 1;
	if ((scoreUpdateTimer / 60) % 2 == 0)
	{
		sprintf_s(scoreUpdateText, "Update!");
	}
	else
	{
		sprintf_s(scoreUpdateText, "");
	}
	count = MakeSpriteString(count, x, y, 12.0f, 12.0f, scoreUpdateText);
}