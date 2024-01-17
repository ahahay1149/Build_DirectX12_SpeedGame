#pragma once

enum DEBUG_FLAG
{
	//1-8 DebugScene
	Scene_All		= 0x00000001,
	Scene_Title		= 0x00000002,
	Scene_InGame	= 0x00000004,
	Scene_GameClear = 0x00000008,
	Scene_GameOver	= 0x00000010,

	//9-16 Shader
	Shader_Stage	= 0x00000200,
	Shader_Player	= 0x00000400,

	//17-24 Component
	Component_All		= 0x00010000,
	Component_HeartItem = 0x00020000,
	Component_Terrain	= 0x00040000,
	Component_Player	= 0x00080000,
};