#pragma once

enum class GAME_SCENES
{
	AWAKE,
	INIT,
	TITLE,
	IN_GAME,
	IN_GAME02,
	IN_GAME03,
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

//===System
namespace System
{
	static constexpr int intFps = 60;
	static constexpr float floatFps = 0.01666f;
}