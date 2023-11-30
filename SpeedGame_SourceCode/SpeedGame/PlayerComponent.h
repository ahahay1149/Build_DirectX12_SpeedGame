#pragma once
#include "GameObject.h"

class PlayerComponent : public GameComponent
{
	//==========HitSystem========
private:
	int m_playerHitpoint;	//プレイヤの体力
	int m_hitDamage;		//このフレームで受けたダメージ

	HitQuad bodyHit;		//プレイヤボディ当たり判定
	//==========HitSystem========

public:
	void initAction() override;
	bool frameAction() override;
	void finishAction() override;

	void hitReaction(GameObject* obj, HitAreaBase* hit) override;
};
