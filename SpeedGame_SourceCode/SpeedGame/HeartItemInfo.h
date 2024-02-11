#pragma once
#include <map>
#include <string>

#include "GamePrograming3Enum.h"

class HeartInfo
{
private:
	//ハートのデータ構造体
	struct Base
	{
		float x, y, z;
		float playerSpeed;
		Heart::TYPE type;
	};

public:
	static const int heartNum = 10;

	static Base heart01[heartNum];
	static Base heart02[heartNum];
	static Base heart03[heartNum];
};

//各種ハートのタイプに合わせて使用するFBXをマッピング
namespace HeartType
{
	static const std::map<Heart::TYPE, std::wstring> typeMap =
	{
		{Heart::TYPE::Gold,	L"GoldenHeart"},
		{Heart::TYPE::Red,	L"RedHeart"},
		{Heart::TYPE::Blue,	L"BlueHeart"},
	};
}

//Stage01
HeartInfo::Base HeartInfo::heart01[HeartInfo::heartNum] =
{
	3.0f, 0.0f, 8.0f,		0.02f,	Heart::TYPE::Gold,	//01
	-11.0f, -4.0f, 10.0f,	0.04f,	Heart::TYPE::Gold,	//02
	13.0f, -3.0f, 22.0f,	0.02f,	Heart::TYPE::Gold,	//03
	0.0f, -6.0f, 20.0f,		0.02f,	Heart::TYPE::Gold,	//04
	-7.0f, -4.0f, 18.0f,	0.02f,	Heart::TYPE::Gold,	//05
	20.0f, 0.0f, 20.0f,		0.02f,	Heart::TYPE::Gold,	//06
	25.0f, 1.0f, 15.0f,		0.03f,	Heart::TYPE::Gold,	//07
	7.0f, -8.0f, 30.0f,		0.01f,	Heart::TYPE::Gold,	//08
	15.0f, 0.0f, 10.0f,		0.02f,	Heart::TYPE::Gold,	//09
	10.0f, 15.0f, 10.0f,	0.1f,	Heart::TYPE::Gold,	//10
};

//Stage02
HeartInfo::Base HeartInfo::heart02[HeartInfo::heartNum] =
{
	-20.0f, -14.0f, 215.0f,	0.04f,	Heart::TYPE::Gold,	//01
	-9.0f, -15.0f, 206.0f,	-0.05f,	Heart::TYPE::Red,	//02
	23.0f, -23.0f, 228.0f,	0.02f,	Heart::TYPE::Gold,	//03
	0.6f, -15.0f, 220.0f,	0.03f,	Heart::TYPE::Gold,	//04
	-10.0f, -13.0f, 224.0f,	0,		Heart::TYPE::Blue,	//05
	25.0f, -20.0f, 216.0f,	0.04f,	Heart::TYPE::Gold,	//06
	15.0f, -16.0f, 200.0f,	0,		Heart::TYPE::Blue,	//07
	9.0f, -18.0f, 230.0f,	-0.05f,	Heart::TYPE::Red,	//08
	20.0f, -18.0f, 207.0f,	-0.05f,	Heart::TYPE::Red,	//09
	10.0f, -16.0f, 210.0f,	0.03f,	Heart::TYPE::Gold,	//10
};

//Stage03
HeartInfo::Base HeartInfo::heart03[HeartInfo::heartNum] =
{
	75.0f, -72.0f, 295.0f,	0.03f,	Heart::TYPE::Gold,	//01
	60.0f, -78.0f, 300.0f,	0,		Heart::TYPE::Blue,	//02
	85.0f, -74.0f, 303.0f,	-0.05f,	Heart::TYPE::Red,	//03
	70.0f, -76.0f, 304.0f,	-0.05f,	Heart::TYPE::Red,	//04
	60.0f, -71.0f, 283.0f,	0,		Heart::TYPE::Blue,	//05
	97.0f, -79.0f, 304.0f,	0,		Heart::TYPE::Blue,	//06
	102.0f, -77.0f, 292.0f,	-0.05f,	Heart::TYPE::Red,	//07
	59.0f, -73.0f, 290.0f,	-0.05f,	Heart::TYPE::Red,	//08
	85.0f, -73.0f, 280.0f,	0,		Heart::TYPE::Blue,	//09
	82.0f, -71.0f, 288.0f,	-0.05f,	Heart::TYPE::Red,	//10
};