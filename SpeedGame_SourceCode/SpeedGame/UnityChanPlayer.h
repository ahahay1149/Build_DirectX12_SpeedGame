﻿#pragma once
#include "GameObject.h"
#include "CameraComponent.h"

//=====動く地形 対応
#include "TerrainComponent.h"
//=====動く地形 対応 END

#include <HitShapes.h>

#include "FBXCharacterData.h"

//ImGui
#include "ImguiComponent.h"

class UnityChanPlayer :
    public GameComponent, public ImguiComponent
{
private:
    float	m_unityChanHeadHeight;  //頭部までのオフセット
    float	m_walkableHeight;	    //足元段差許容位置

    //Unityちゃん　飛ぶ
    float   m_jumpPower;
    float   m_gravityPower;
    float   m_terminalVelocity;
    float   m_YSpeed;

    bool    m_onGround; //地面に接地しているかを判定

    CameraComponent* m_currentCamera;

    //Unityちゃん本体のヒット判定
    HitPillar bodyColl;
    float m_hitHeight;

    //=====動く地形 対応
    TerrainComponent* m_currentTerrain;
    XMMATRIX m_lastMatrix;  //最後に変換した行列
    //=====動く地形 対応 END

     //ハートアイテムの獲得数
    int m_getHeartItems = 0;

    //UnityChanの加速度
    float m_plusSpeed;

public:
    // GameComponent を介して継承されました
    virtual void initAction() override;
    virtual bool frameAction() override;
    virtual void finishAction() override;

    //Unityちゃん本体のヒットリアクション
    virtual void hitReaction(GameObject* targetGo, HitAreaBase* hit) override;

    void SetCurrentCamera(CameraComponent* cam)
    {
        m_currentCamera = cam;
    }

    //ハートにある値分スピードを足す
    void setSpeedCount(float speed)
    {
        m_plusSpeed += speed;
    }

    //ImGui
    void imgui() override;

    XMFLOAT3 m_initPos;
    XMFLOAT3 imguiPos = { 0,0,0 };
};
