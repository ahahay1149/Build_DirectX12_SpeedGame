#include "MyAccessHub.h"
#include "PlayerComponent.h"

#include "SpriteCharacter.h"
#include "GamePrograming3Enum.h"

#include <DirectXMath.h>

#define PLAYER_MOVE_SPEED (2.0f)
#define PLAYER_45_SPEED (PLAYER_MOVE_SPEED / (1.41421356f))

using namespace DirectX;

void PlayerComponent::initAction()
{
	//SpriteCharacterを設定
	SpriteCharacter* sc = (SpriteCharacter*)getGameObject()->getCharacterData();
	sc->setTextureId(L"Sprite00");
	sc->SetGraphicsPipeLine(L"AlphaSprite");	//アルファブレンドスプライト

	XMUINT4 patterns[1] = {
		{0, 0, 20, 24}
	};

	sc->setSpritePatterns(patterns, 1);

	//==========HitSystem========
	m_playerHitpoint = 1;
	m_hitDamage = 0;

	bodyHit.setAttackType(static_cast<UINT>(HIT_ORDER::HIT_PLAYER_BODY), m_playerHitpoint);	//ヒットタイプはPLAYER＿BODY
	bodyHit.setHitSize(10.0f, 12.0f);			//描画時にテクスチャサイズから0.1倍しているので注意	
	//==========HitSystem========
}

void PlayerComponent::finishAction()
{
	//とりあえず特に今回は中身がない
}

enum DIRECTION_CODE
{
	MOVE_UP = 0x00000001,
	MOVE_DOWN = 0x00000002,
	MOVE_LEFT = 0x00000004,
	MOVE_RIGHT = 0x00000008,

	MOVE_UPLEFT = 0x00000005,
	MOVE_UPRIGHT = 0x00000009,
	MOVE_DOWNLEFT = 0x00000006,
	MOVE_DOWNRIGHT = 0x0000000a,
};

bool PlayerComponent::frameAction()
{
	MyGameEngine* pEngine = MyAccessHub::getMyGameEngine();
	CharacterData* pChara = getGameObject()->getCharacterData();
	InputManager* pInput = pEngine->GetInputManager();
	HitManager* pHit = pEngine->GetHitManager();

	byte pressFlg = 0;					//方向キープレスフラグ　0で何も押されていない

	m_playerHitpoint -= m_hitDamage;	//ダメージ計算
	m_hitDamage = 0;					//このフレームのダメージ値リセット

	if (m_playerHitpoint < 1)			//耐久度が0になったのでプレイヤアウト
	{
		//pEngine->GetSoundManager()->play(1);
		return false;
	}

	//今回はここに記述、後にコンフィグクラスなど作って管理したい(要修正)
	if (pInput->checkKeyboard(VK_UP, InputManager::BUTTON_STATE::BUTTON_PRESS))				//キーボードの上下左右をチェックし、押されていたキーのフラグをビット和する
	{
		pressFlg = DIRECTION_CODE::MOVE_UP;
	}
	else if (pInput->checkKeyboard(VK_DOWN, InputManager::BUTTON_STATE::BUTTON_PRESS))
	{
		pressFlg = DIRECTION_CODE::MOVE_DOWN;
	}

	if (pInput->checkKeyboard(VK_LEFT, InputManager::BUTTON_STATE::BUTTON_PRESS))
	{
		pressFlg |= DIRECTION_CODE::MOVE_LEFT;
	}
	else if (pInput->checkKeyboard(VK_RIGHT, InputManager::BUTTON_STATE::BUTTON_PRESS))
	{
		pressFlg |= DIRECTION_CODE::MOVE_RIGHT;
	}

	XMFLOAT3 pos = pChara->getPosition();		//現在の座標値取得。キーフラグで座標値に加算する

	switch (pressFlg)
	{
	case DIRECTION_CODE::MOVE_UP:
		pos.y += PLAYER_MOVE_SPEED;
		break;

	case DIRECTION_CODE::MOVE_DOWN:
		pos.y -= PLAYER_MOVE_SPEED;
		break;

	case DIRECTION_CODE::MOVE_LEFT:
		pos.x -= PLAYER_MOVE_SPEED;
		break;

	case DIRECTION_CODE::MOVE_RIGHT:
		pos.x += PLAYER_MOVE_SPEED;
		break;

	case DIRECTION_CODE::MOVE_UPLEFT:			//ここから斜め移動。PLAYER_MOVE_SPEEDのままだと斜めの方が移動が速くなる
		pos.x -= PLAYER_45_SPEED;
		pos.y += PLAYER_45_SPEED;
		break;

	case DIRECTION_CODE::MOVE_UPRIGHT:
		pos.x += PLAYER_45_SPEED;
		pos.y += PLAYER_45_SPEED;
		break;

	case DIRECTION_CODE::MOVE_DOWNLEFT:
		pos.x -= PLAYER_45_SPEED;
		pos.y -= PLAYER_45_SPEED;
		break;

	case DIRECTION_CODE::MOVE_DOWNRIGHT:
		pos.x += PLAYER_45_SPEED;
		pos.y -= PLAYER_45_SPEED;
		break;
	}

	pChara->setPosition(pos.x, pos.y, pos.z);

	//移動後の座標に判定を設定する
	bodyHit.setAttackType(static_cast<UINT>(HIT_ORDER::HIT_PLAYER_BODY), m_playerHitpoint);	//貫通系対応
	bodyHit.setPosition(pos.x, pos.y);											//位置修正（サイズはそのまま）

	pHit->setHitArea(this, &bodyHit);	//当たり判定をシステムにセット

	//PipeLineに登録
	pChara->GetPipeline()->AddRenerObject(pChara);
	
	return true;
}

//==========HitSystem========
void PlayerComponent::hitReaction(GameObject* obj, HitAreaBase* hit)
{
	switch (hit->getHitType())
	{
	case static_cast<UINT>(HIT_ORDER::HIT_ENEMY_ATTACK):	//敵の攻撃を受けた
		m_hitDamage += hit->getHitPower();	//フレームダメージ値に攻撃力を加算
		break;

	default:
		break;
	}
}
//==========HitSystem========
