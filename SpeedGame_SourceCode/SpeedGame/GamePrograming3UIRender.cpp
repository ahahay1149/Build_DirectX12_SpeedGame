#include "GamePrograming3UIRender.h"

#include <MyAccessHub.h>
#include <D3D12Helper.h>

#include "GamePrograming3Scene.h"
#include "GameAccessHub.h"
#include "GamePrograming3Enum.h"

void GamePrograming3UIRender::initAction()
{
	UIRenderBase::initAction();
}

bool GamePrograming3UIRender::frameAction()
{
	MyGameEngine* engine = MyAccessHub::getMyGameEngine();
	GraphicsPipeLineObjectBase* pipeLine = engine->GetPipelineManager()->GetPipeLineObject(L"Sprite");

	int count = 0;

	//中心0,0 w960 h540
	float x = -480.0f + 30.0f;	//-450.0f
	float y = 270.0f - 30.0f;	// 240.0f

	//開始前のカウントダウンのUI制御
	int startCount = GameAccessHub::getGameManager()->getStartCount();
	switch (startCount)
	{
	case 3:
		sprintf_s(countText, "  %d", 3);
		break;
	case 2:
		sprintf_s(countText, "  %d", 2);
		break;
	case 1:
		sprintf_s(countText, "  %d", 1);
		break;
	case 0:
		sprintf_s(countText, "Start!");
		break;

	default:
		sprintf_s(countText, "");
		break;
	}

	GamePrograming3Scene* scene = static_cast<GamePrograming3Scene*>(engine->GetSceneController());
	GameManager* gm = GameAccessHub::getGameManager();

	//ゲーム中のカウントダウンUI制御
	float timerCount = gm->getTimerCount();
	sprintf_s(timerText, "Time %.3f", timerCount);

	//クリアまでの取得カウント(ステージごとに変更)
	int stageClearCount = 0;
	switch (scene->getCurrentScene())
	{
	case static_cast<UINT>(GAME_SCENES::IN_GAME):
		stageClearCount = (int)Game::CLEAR_COUNT::stage01;
		break;
	case static_cast<UINT>(GAME_SCENES::IN_GAME02):
		stageClearCount = (int)Game::CLEAR_COUNT::stage02;
		break;
	case static_cast<UINT>(GAME_SCENES::IN_GAME03):
		stageClearCount = (int)Game::CLEAR_COUNT::stage03;
		break;
	}

	//クリアまでのカウントを表示
	int heartItemCount = gm->getHeartItemCount();
	sprintf_s(itemText, "Point %d / %d", heartItemCount, stageClearCount);

	//速度の増加量を表示
	sprintf_s(playerSpeedText, "Speed +%.0f%%", gm->getPlayerSpeed() * 1000);

	count = MakeSpriteString(count, x, y, 24, 24, itemText);
	count = MakeSpriteString(count, x, y - 40, 24, 24, timerText);
	count = MakeSpriteString(count, x, y - 80, 24, 24, playerSpeedText);

	count = MakeSpriteString(count, x + 350, y - 240, 48, 48, countText);

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