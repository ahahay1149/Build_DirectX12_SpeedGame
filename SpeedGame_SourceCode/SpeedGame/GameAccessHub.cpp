#include "GameAccessHub.h"

//GameManager等のアプリケーションシステム系の
//アクセスを管理するクラス
//MyAccessHubではengineのハブとして機能させ、
//こちらではクライアント系のシステムを登録する

GameManager*				GameAccessHub::m_gameManager = nullptr;
UnityChanPlayer*			GameAccessHub::m_player = nullptr;