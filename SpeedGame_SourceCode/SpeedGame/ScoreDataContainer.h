#pragma once
#include "GameAppEnum.h"

class ScoreDataContainer
{
private:
	struct Stage
	{
		float m_clearTime;
		float m_bestTime;
		bool m_updateTime;
	};

	static float m_startStageTime;

	static void setResultTime();

public:
	static Stage stage[int(Game::STAGE_NUM::Count) + 1];	//GameScene + Result

	static void setClearTime(float clearTime, Game::STAGE_NUM stage);

	static void clearUpdateTime()
	{
		for (int i = 0; i < sizeof(stage) / sizeof(Stage); i++)
		{
			stage[i].m_clearTime = 0.0f;
			stage[i].m_updateTime = false;
		}
	}

	static void setStartStageTime(float startTime)
	{
		m_startStageTime = startTime;
	}
};