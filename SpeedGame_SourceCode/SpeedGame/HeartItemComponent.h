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

    //ハートが持つ移動速度の上昇量を保存
    float       m_heartSpeed;

    //ハートタイプに応じたポイントを保存
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

    void setHeartSpeed(float heartSpeed)
    {
        m_heartSpeed = heartSpeed;
    }

    void setHeartType(int heartType)
    {
        m_heartPoint = heartType;
    }

    //ImGui
    void imgui() override;
    void imguiInit() override;

    XMFLOAT3 m_initPos;
    XMFLOAT3 m_savePos;
};

