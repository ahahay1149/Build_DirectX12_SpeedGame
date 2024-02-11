#include "ScoreDataContainer.h"

ScoreDataContainer::Stage ScoreDataContainer::stage[int(Game::STAGE_NUM::Count) + 1]
{
  //Clear //Best
	0.0f, 0.0f,	false,	//Stage01
	0.0f, 0.0f,	false,	//Stage02
	0.0f, 0.0f,	false,	//Stage03
	0.0f, 0.0f,	false,	//Result
};

float ScoreDataContainer::m_startStageTime = Timer::Game_Count;

void ScoreDataContainer::setClearTime(float clearTime, Game::STAGE_NUM stageNum)
{
	float clearCount = m_startStageTime - clearTime;
	int sceneNum = int(stageNum);

	//BestTime判定
	if (clearCount < stage[sceneNum].m_bestTime || stage[sceneNum].m_bestTime == 0.0f)
	{
		stage[sceneNum].m_bestTime = clearCount;
		stage[sceneNum].m_updateTime = true;
	}

	//ClearTime入力
	stage[sceneNum].m_clearTime = clearCount;

	//Resultの1つ前のシーンで処理を行えば自動的に最後のステージの処理も行う
	if (sceneNum == int(Game::STAGE_NUM::Count) - 1)
		setResultTime();
}

void ScoreDataContainer::setResultTime()
{
	int count = int(Game::STAGE_NUM::Count);
	float clearCount = 0;

	//全ステージのクリアタイムを集計
	for (int i = 0; i < count; i++)
	{
		clearCount += stage[i].m_clearTime;
	}

	//BestTime判定
	if (clearCount < stage[count].m_bestTime || stage[count].m_bestTime == 0.0f)
	{
		stage[count].m_bestTime = clearCount;
		stage[count].m_updateTime = true;
	}

	//ClearTime入力
	stage[count].m_clearTime = clearCount;
}

