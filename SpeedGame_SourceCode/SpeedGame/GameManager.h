#pragma once
#include "GameObject.h"

#include "HeartItemComponent.h"
#include "UnityChanPlayer.h"

#include "GameAppEnum.h"

#include "ImguiComponent.h"

class GameManager :
    public GameComponent, public ImguiComponent
{
private:
    //===Player
    int m_heartItemCount = 0;   //取得したハートのカウント
    float m_playerSpeed = 0.0f; //Playerの取得したスピード
    //=========

    //===Time
    float timerCount = Timer::Game_Count;       //ゲームの制限時間
    int startCount = Timer::Start_Count;     //カウントダウンの時間
    //========

    //===Sound
    // 再生を行っている音のidを格納する
    int m_playingMusic[3];  //0:InGame //1:GameClear //2:GameOver
    //========
    
    //===Scene
    UINT m_scene;   //現在のシーン
    //========

    void changeSceneInit(UINT scene);

public:

    //frameActionよりも前で処理を行う
    void sendScene(UINT scene);

    // GameComponent を介して継承されました
    virtual void initAction() override;
    virtual bool frameAction() override;
    virtual void finishAction() override;

    //===Timer Accessor
    int getStartCount()
    {
        return startCount / System::intFps;
    }

    float getTimerCount()
    {
        return timerCount;
    }
    //===Timer Accessor End

    //===Player Accessor
    void setPlusHeartItemCount(int heartCount)
    {
        m_heartItemCount += heartCount;
    }

    int getHeartItemCount()
    {
        return m_heartItemCount;
    }

    float getPlayerSpeed()
    {
        return m_playerSpeed;
    }

    void varSetPlayerSpeed(float heartSpeed);
    void varSetPlayerSpeed(int blueHeart);
    //===Player Accessor End

private:
    //ImGui
    void imgui() override;
    void imguiCountRegist();

    bool timerStop = false;
    bool countStop = false;
    float count = 0.0f;

    int selectScene = -1;
    const char* sceneItems[6] = { "Title", "In_Game", "In_Game02", "In_Game03", "Game_Over", "Game_Clear" };
};