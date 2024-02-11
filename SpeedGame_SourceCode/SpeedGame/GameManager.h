#pragma once
#include "GameObject.h"
#include "HeartItemComponent.h"
#include "ImguiComponent.h"
#include "UnityChanPlayer.h"

constexpr float Max_Count   = 30.0f;
constexpr float Start_Count = 4.0f;

class GameManager :
    public GameComponent, public ImguiComponent
{
private:
    //===Player
    int m_heartItemCount;   //取得したハートのカウント
    //=========

    //===Time
    float timerCount = Max_Count;       //ゲームの制限時間
    float startCount = Start_Count;     //カウントダウンの時間
    //========

    //===Score
    float bestScore = 0.0f;             //ゲーム全体でのベストタイム
    float clearScore = 0.0f;            //クリア時の残り時間
    //========

    //===Sound
    // 再生を行っている音のidを格納する
    int m_playingMusic[3];  //0:InGame //1:GameClear //2:GameOver
    //========
    
    //===Scene
    UINT m_scene;   //現在のシーン
    //========

    void changeSceneInit(UINT scene);
    void countTimer();

public:
    // GameComponent を介して継承されました
    virtual void initAction() override;
    virtual bool frameAction() override;
    virtual void finishAction() override;

    void sendScene(UINT scene);

    float getStartCount()
    {
        return startCount;
    }

    float getTimerCount()
    {
        return timerCount;
    }

    float getclearScore()
    {
        return clearScore;
    }

    float getBestScore()
    {
        return bestScore;
    }

    int getHeartItemCount()
    {
        return m_heartItemCount;
    }

    void setPlusHeartItemCount(int heartCount)
    {
        m_heartItemCount += heartCount;
    }

private:
    //ImGui
    void imgui() override;

    bool timerStop = false;
    float count = 0.0f;

    int selectScene = -1;
    const char* sceneItems[4] = { "Title","In_Game","Game_Over", "Game_Clear" };
};