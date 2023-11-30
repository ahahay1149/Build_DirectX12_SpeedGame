#pragma once
#include "GameObject.h"

#include <HitShapes.h>

class HeartItemComponent :
    public GameComponent
{
private:
    HitSphere   m_itemHit;
    float       m_centerY;
    int         sta;

    //プレイヤーの上昇量データを保存
    float       m_playerSpeed;

    //回転用の座標データ
    XMFLOAT3    rotation;

public:
    // GameComponent を介して継承されました
    virtual void initAction() override;
    virtual bool frameAction() override;
    virtual void finishAction() override;

    virtual void hitReaction(GameObject* targetGo, HitAreaBase* hit) override;

    void setPlayerSpeed(float playerSpeed);
    float getPlayerSpeed();
};

