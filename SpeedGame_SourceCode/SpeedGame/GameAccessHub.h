#pragma once

#include "GameManager.h"
#include "UnityChanPlayer.h"
class GameManager;
class UnityChan;

class GameAccessHub
{
private:
	static GameManager* m_gameManager;
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

