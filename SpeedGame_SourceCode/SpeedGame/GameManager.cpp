#include "GameManager.h"
#include "GameAccessHub.h"
#include "MyAccessHub.h"

#include "GamePrograming3Scene.h"
#include "GamePrograming3Enum.h"

void GameManager::initAction()
{

}

bool GameManager::frameAction()
{
	//Update
	switch (m_scene)
	{
	case static_cast<UINT>(GAME_SCENES::IN_GAME):
		{
			//インゲーム内のタイマー
			countTimer();

			//HeartItemを一定数取得したらゲームクリアでシーン遷移
			if (heartItem >= 5)
			{
				MyAccessHub::getMyGameEngine()->
						GetSceneController()->OrderNextScene((UINT)GAME_SCENES::GAME_CLEAR);
			}
		}
		break;
	case static_cast<UINT>(GAME_SCENES::GAME_CLEAR):
		{
			//clearScoreに保存
			clearScore = timerCount;
			//現在のタイムとベストタイムを比較し、タイムの上書きを行う
			if (clearScore > bestScore)
			{
				bestScore = clearScore;
			}
		}
		break;
	}

	return true;
}

//シーン変更時に一度だけ呼び出しを行う
void GameManager::ChangeSceneInit(UINT scene)
{
	MyGameEngine* engine = MyAccessHub::getMyGameEngine();
	SoundManager* soMng = engine->GetSoundManager();

	switch (scene)
	{
	case static_cast<UINT>(GAME_SCENES::IN_GAME):
	{
		//各種パラメータを初期化
		startCount = 4.0f;
		timerCount = 30.0f;
		heartItem  = 0;

		//InGame BGM
		soMng->stop(playingMusic[1]);
		soMng->stop(playingMusic[2]);
		playingMusic[0] = soMng->play(0);
	}
	break;
	case static_cast<UINT>(GAME_SCENES::GAME_CLEAR):
	{
		soMng->stop(playingMusic[0]);
		playingMusic[2] = soMng->play(1);
	}
	break;
	case static_cast<UINT>(GAME_SCENES::GAME_OVER):
	{
		soMng->stop(playingMusic[0]);
		playingMusic[1] = soMng->play(2);
	}
	break;
	}
}

void GameManager::finishAction()
{

}

void GameManager::countTimer()
{
	if (startCount > 0)
	{
		startCount -= 0.01666f;
	}
	else if(timerCount > 0 && startCount <= 0)
	{
		timerCount -= 0.01666f;
	}
	else if (timerCount < 0 && startCount <= 0)
	{
		MyAccessHub::getMyGameEngine()->GetSceneController()->OrderNextScene((UINT)GAME_SCENES::GAME_OVER);
	}
}

void GameManager::sendScene(UINT scene)
{
	//送る側と受け取る側が同じシーンだった場合処理を行わない
	if (m_scene == scene)
		return;

	m_scene = scene;
	ChangeSceneInit(scene);
}