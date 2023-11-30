﻿#pragma once

enum class GAME_SCENES
{
	AWAKE,
	INIT,
	TITLE,
	IN_GAME,
	GAME_OVER,

	//=========Change Scene
	GAME_CLEAR,
	//=========Change Scene END

};

enum class HIT_ORDER
{
	HIT_PLAYER_BODY,
	HIT_PLAYER_ATTACK,
	HIT_PLAYER_SHIELD,
	HIT_ENEMY_BODY,
	HIT_ENEMY_ATTACK,
	HIT_ENEMY_SHIELD,
	HIT_ITEM,
};

