#pragma once

#include "MyGameEngine.h"

class MyGameEngine;

class MyAccessHub
{
private:
	static MyGameEngine* m_engine;

public:
	static void setMyGameEnegine(MyGameEngine* eng)
	{
		m_engine = eng;
	}

	static MyGameEngine* getMyGameEngine()
	{
		return m_engine;
	}
};
