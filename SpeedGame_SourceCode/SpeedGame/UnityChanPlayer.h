#pragma once
#include "GameObject.h"
#include "CameraComponent.h"

//=====動く地形 対応
#include "TerrainComponent.h"
//=====動く地形 対応 END

#include <HitShapes.h>

#include "FBXCharacterData.h"

class UnityChanPlayer :
    public GameComponent
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

    //ImGui
    bool check = false;
    int selectShader = -1;
    const char* items[4] = { "Lambert","Phong","Blinn Phong", "Toon"};

    void imgui(FBXCharacterData* chData);

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
    void plusSpeedCount(float speed)
    {
        m_plusSpeed += speed;
    }
};
