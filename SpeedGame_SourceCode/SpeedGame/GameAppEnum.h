#pragma once

//===Timer
namespace Timer
{
	//インゲームの制限時間
	constexpr float Game_Count = 90.0f;
	//カウントダウンの時間
	constexpr int Start_Count = 240;	// Start_Count / 60 = RealSecond
}

//===Player
namespace Player
{
	//速度上限
	constexpr float Max_Speed = 0.2f;
	//速度下限
	constexpr float Min_Speed = 0.0f;

	struct Position
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
	};

	//各ステージの初期位置
	static const Position stagePos[3] =
	{
		{0.0f,	0.0f,	0.0f},
		{0.0f,	-14.9f, 190.0f},
		{70.0f,-70.0f,	275.0f},
	};
}

//===Heart
namespace Heart
{
	//ハートのタイプ
	enum class TYPE
	{
		Gold = 1,
		Red = 3,
		Blue = 5,
	};
}

//===Game
namespace Game
{
	enum class STAGE_NUM
	{
		IN_GAME01,
		IN_GAME02,
		IN_GAME03,
		Count,		//ステージ数
	};

	//各ステージがクリアに必要なスコア数
	enum class CLEAR_COUNT
	{
		stage01 = 5,
		stage02 = 8,
		stage03 = 15,
	};
}