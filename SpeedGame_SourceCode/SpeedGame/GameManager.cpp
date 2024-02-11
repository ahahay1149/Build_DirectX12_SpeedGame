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
			//ImGui処理
			if (timerStop != true)
			{
				countTimer();
			}

			//HeartItemを一定数取得したらゲームクリアでシーン遷移
			if (m_heartItemCount >= 5)
			{
				MyAccessHub::getMyGameEngine()->
						GetSceneController()->OrderNextScene((UINT)GAME_SCENES::GAME_CLEAR);
			}
		}
		break;
	}

	return true;
}

//シーン変更時に一度だけ呼び出しを行う
void GameManager::changeSceneInit(UINT scene)
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
			m_heartItemCount  = 0;

			//InGame BGM
			soMng->stop(m_playingMusic[1]);
			soMng->stop(m_playingMusic[2]);
			m_playingMusic[0] = soMng->play(0);
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

			//GameClear SE
			soMng->stop(m_playingMusic[0]);
			m_playingMusic[1] = soMng->play(1);
		}
		break;
		case static_cast<UINT>(GAME_SCENES::GAME_OVER):
		{
			//GameOver SE
			soMng->stop(m_playingMusic[0]);
			m_playingMusic[2] = soMng->play(2);
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
	changeSceneInit(scene);
}

void GameManager::imgui()
{
	ImGui::Begin("Window");
	ImGui::Checkbox("GameManager", &check);
	ImGui::End();

	if (check == true)
	{
		ImGui::Begin("GameManager");
		ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_::ImGuiCond_FirstUseEver);

		ImGui::SeparatorText("InGame");
		//Timer Edit & Timer Stop
		ImGui::SliderFloat("Timer", &timerCount, Max_Count, 0.0f);
		ImGui::Checkbox("Timer Stop", &timerStop);

		ImGui::SeparatorText("Sound");
		//Stop Music
		if (ImGui::Button("Stop InGameBGM"))
			MyAccessHub::getMyGameEngine()->GetSoundManager()->stop(m_playingMusic[0]);

		ImGui::SeparatorText("ChangeScene");

		if (ImGui::Combo("ChangeScene", &selectScene, sceneItems, IM_ARRAYSIZE(sceneItems)))
		{
			switch (selectScene)
			{
			case 0:
				MyAccessHub::getMyGameEngine()->GetSceneController()->OrderNextScene((UINT)GAME_SCENES::TITLE);
				break;
			case 1:
				MyAccessHub::getMyGameEngine()->GetSceneController()->OrderNextScene((UINT)GAME_SCENES::IN_GAME);
				break;
			case 2:
				MyAccessHub::getMyGameEngine()->GetSceneController()->OrderNextScene((UINT)GAME_SCENES::GAME_OVER);
				break;
			case 3:
				MyAccessHub::getMyGameEngine()->GetSceneController()->OrderNextScene((UINT)GAME_SCENES::GAME_CLEAR);
				break;
			}
		}

		ImGui::End();
	}
}