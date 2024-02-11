#include "GameManager.h"
//AccessHub
#include "GameAccessHub.h"
#include "MyAccessHub.h"

#include "GamePrograming3Scene.h"
#include "GamePrograming3Enum.h"

#include "ScoreDataContainer.h"

void GameManager::initAction()
{
}

bool GameManager::frameAction()
{
	//インゲーム共通処理
	switch (m_scene)
	{
		case static_cast<UINT>(GAME_SCENES::IN_GAME):
		case static_cast<UINT>(GAME_SCENES::IN_GAME02):
		case static_cast<UINT>(GAME_SCENES::IN_GAME03):
		{
			//ImGui処理
			if (timerStop != true)
			{
				//インゲーム内タイマー
				MyGameEngine* engine = MyAccessHub::getMyGameEngine();
				SoundManager* soMng = engine->GetSoundManager();

				if (startCount >= 0)
				{
					switch (startCount)
					{
					case Timer::Start_Count:
						ScoreDataContainer::setStartStageTime(timerCount);
					case 180:
					case 120:
						soMng->play(3);
						break;
					case 60:
						m_playingMusic[0] = soMng->play(4);
						break;
					case 5:
						soMng->stop(m_playingMusic[0]);
						break;
					case 1:
						//各シーンに合わせたBGMを鳴らす
						switch (m_scene)
						{
						case static_cast<UINT>(GAME_SCENES::IN_GAME):
							m_playingMusic[0] = soMng->play(0);
							break;
						case static_cast<UINT>(GAME_SCENES::IN_GAME02):
							m_playingMusic[0] = soMng->play(1);
							break;
						case static_cast<UINT>(GAME_SCENES::IN_GAME03):
							m_playingMusic[0] = soMng->play(2);
							break;
						}
						break;
					case 0:
						startCount = System::intFps * -1;	//fpsで割っても-1を返すように
					}

					startCount -= 1;
				}
				else if (timerCount >= 0 && startCount <= 0)
				{
					timerCount -= System::floatFps;
				}
				else if (timerCount < 0 && startCount <= 0)
				{
					//ゲームの制限時間が0になるとゲームオーバーシーンに遷移
					MyAccessHub::getMyGameEngine()->GetSceneController()->OrderNextScene((UINT)GAME_SCENES::GAME_OVER);
				}
			}

			//スピード上昇分をセットする
			GameAccessHub::getUnityChan()->getPlayerPlusSpeed(m_playerSpeed);
		}
		break;
	}

	//インゲーム個別処理
	switch (m_scene)
	{
	case static_cast<UINT>(GAME_SCENES::IN_GAME):
	{
		//HeartItemを一定数取得したらシーン遷移
		if (m_heartItemCount >= (int)Game::CLEAR_COUNT::stage01)
		{
			ScoreDataContainer::setClearTime(timerCount, Game::STAGE_NUM::IN_GAME01);

			MyAccessHub::getMyGameEngine()->
				GetSceneController()->OrderNextScene((UINT)GAME_SCENES::GAME_CLEAR);
		}
	}
	break;
	case static_cast<UINT>(GAME_SCENES::IN_GAME02):
	{
		//HeartItemを一定数取得したらシーン遷移
		if (m_heartItemCount >= (int)Game::CLEAR_COUNT::stage02)
		{
			ScoreDataContainer::setClearTime(timerCount, Game::STAGE_NUM::IN_GAME02);

			MyAccessHub::getMyGameEngine()->
				GetSceneController()->OrderNextScene((UINT)GAME_SCENES::GAME_CLEAR);
		}
	}
	break;
	case static_cast<UINT>(GAME_SCENES::IN_GAME03):
	{
		//HeartItemを一定数取得したらシーン遷移
		if (m_heartItemCount >= (int)Game::CLEAR_COUNT::stage03)
		{
			ScoreDataContainer::setClearTime(timerCount, Game::STAGE_NUM::IN_GAME03);

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

	soMng->stop(m_playingMusic[0]);

	switch (scene)
	{
		case static_cast<UINT>(GAME_SCENES::IN_GAME):
		{
			//各種パラメータを初期化
			startCount = Timer::Start_Count;
			timerCount = Timer::Game_Count;
			m_heartItemCount  = 0;
			m_playerSpeed = Player::Min_Speed;
			ScoreDataContainer::clearUpdateTime();

			//InGame BGM
			soMng->stop(m_playingMusic[1]);
			soMng->stop(m_playingMusic[2]);
		}
		break;
		case static_cast<UINT>(GAME_SCENES::IN_GAME02):
		case static_cast<UINT>(GAME_SCENES::IN_GAME03):
		{
			//各種パラメータを初期化
			startCount = Timer::Start_Count;
			m_heartItemCount = 0;
		}
		break;
		case static_cast<UINT>(GAME_SCENES::GAME_CLEAR):
		{
			//GameClear SE
			//前のシーンがIN_GAME03だった場合、リザルトのジングルを変更
			if (m_scene == (UINT)GAME_SCENES::IN_GAME03)
				m_playingMusic[1] = soMng->play(6);
			else
				m_playingMusic[1] = soMng->play(5);
		}
		break;
		case static_cast<UINT>(GAME_SCENES::GAME_OVER):
		{
			//GameOver SE
			m_playingMusic[2] = soMng->play(7);
		}
		break;
	}
}

void GameManager::finishAction()
{

}

void GameManager::sendScene(UINT scene)
{
	//送る側と受け取る側が同じシーンだった場合処理を行わない
	if (m_scene == scene)
		return;

	changeSceneInit(scene);

	//Initの処理が完了してからシーン識別情報を同期させる
	m_scene = scene;
}

//======Player Accessor
void GameManager::varSetPlayerSpeed(float heartSpeed)
{
	m_playerSpeed += heartSpeed;

	//上限と下限をチェックし、超えていたら補正
	if (m_playerSpeed >= Player::Max_Speed)
		m_playerSpeed = Player::Max_Speed;
	else if (m_playerSpeed <= Player::Min_Speed)
		m_playerSpeed = Player::Min_Speed;
}

void GameManager::varSetPlayerSpeed(int blueHeart)
{
	m_playerSpeed = Player::Min_Speed;
}
//======Player Accessor End

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
		ImGui::SliderFloat("Timer", &timerCount, Timer::Game_Count, 0.0f);
		ImGui::Checkbox("Timer Stop", &timerStop);

		//Sound Stop
		ImGui::SeparatorText("Sound");
		if (ImGui::Button("Stop InGameBGM"))
			MyAccessHub::getMyGameEngine()->GetSoundManager()->stop(m_playingMusic[0]);

		//Clear Count
		ImGui::SeparatorText("ClearPoint");
		ImGui::InputInt("ClearPoint", &m_heartItemCount);
		ImGui::Checkbox("ClearPoint CountStop", &countStop);
		if (countStop == true)
			m_heartItemCount = 0;

		//Scene Change
		ImGui::SeparatorText("ChangeScene");
		if (ImGui::Combo("ChangeScene", &selectScene, sceneItems, IM_ARRAYSIZE(sceneItems)) || ImGui::Button("SameSceneChange"))
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
				MyAccessHub::getMyGameEngine()->GetSceneController()->OrderNextScene((UINT)GAME_SCENES::IN_GAME02);
				break;
			case 3:
				MyAccessHub::getMyGameEngine()->GetSceneController()->OrderNextScene((UINT)GAME_SCENES::IN_GAME03);
				break;
			case 4:
				MyAccessHub::getMyGameEngine()->GetSceneController()->OrderNextScene((UINT)GAME_SCENES::GAME_OVER);
				break;
			case 5:
				MyAccessHub::getMyGameEngine()->GetSceneController()->OrderNextScene((UINT)GAME_SCENES::GAME_CLEAR);
				imguiCountRegist();
				break;
			}
		}
		ImGui::End();
	}
}

void GameManager::imguiCountRegist()
{
	switch (m_scene)
	{
	case static_cast<UINT>(GAME_SCENES::IN_GAME):
		ScoreDataContainer::setClearTime(timerCount, Game::STAGE_NUM::IN_GAME01);
		break;

	case static_cast<UINT>(GAME_SCENES::IN_GAME02):
		ScoreDataContainer::setClearTime(timerCount, Game::STAGE_NUM::IN_GAME02);
		break;

	case static_cast<UINT>(GAME_SCENES::IN_GAME03):
		ScoreDataContainer::setClearTime(timerCount, Game::STAGE_NUM::IN_GAME03);
		break;

	}
}