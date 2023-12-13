#pragma once

#include "GameManager.h"
#include "GamePrograming3UIRender.h"
#include "ResultUIRender.h"
#include "UnityChanPlayer.h"

class GameManager;
class GamePrograming3UIRender;
class ResultUIRender;
class UnityChan;

class GameAccessHub
{
private:
	static GameManager* m_gameManager;
	static GamePrograming3UIRender* m_inGameUIRender;
	static ResultUIRender* m_resultUIRender;
	static UnityChanPlayer* m_player;

public:
	//GameManager
	static void setGameManager(GameManager* gm)
	{
		m_gameManager = gm;
	}

	static GameManager* getGameManager()
	{
		return m_gameManager;
	}

	//InGameUIRender
	static void setInGameUIRender(GamePrograming3UIRender* InGameUIRender)
	{
		m_inGameUIRender = InGameUIRender;
	}

	static GamePrograming3UIRender* getInGameUIRender()
	{
		return m_inGameUIRender;
	}

	//ResultUIRender
	static void setResultUIRender(ResultUIRender* resultUIRender)
	{
		m_resultUIRender = resultUIRender;
	}

	static ResultUIRender* getResultUIRender()
	{
		return m_resultUIRender;
	}

	//UnityChanPlayer
	static void setUnityChan(UnityChanPlayer* player)
	{
		m_player = player;
	}

	static UnityChanPlayer* getUnityChan()
	{
		return m_player;
	}
};

