#pragma once
#include "GameObject.h"
#include "ImguiComponent.h"

#include <HitShapes.h>

class HeartItemComponent :
    public GameComponent, public ImguiComponent
{
private:
    HitSphere   m_itemHit;
    float       m_centerY;

    //プレイヤーの上昇量データを保存
    float       m_playerSpeed;

    //ハートあたりのポイントを保存
    int         m_heartPoint;

    //回転用の座標データ
    XMFLOAT3    m_rotation;

    //移動用の座標データ
    XMFLOAT3    m_pos;

public:
    // GameComponent を介して継承されました
    virtual void initAction() override;
    virtual bool frameAction() override;
    virtual void finishAction() override;

    virtual void hitReaction(GameObject* targetGo, HitAreaBase* hit) override;

    void setPlayerSpeed(float playerSpeed)
    {
        m_playerSpeed = playerSpeed;
    }

    float getPlayerSpeed()
    {
        return m_playerSpeed;
    }

    void setHeartPoint(int heartPoint)
    {
        m_heartPoint = heartPoint;
    }

    int getHeartPoint()
    {
        return m_heartPoint;
    }

    //ImGui
    void imgui() override;
    void imguiInit() override;

    XMFLOAT3 m_initPos;
};

