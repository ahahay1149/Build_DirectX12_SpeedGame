#pragma once
#include "GameObject.h"

class GameManager : public GameComponent
{
private:
    //===Player
    int heartItem;  //取得したハートのカウント
    //========

    //===Time
    float timerCount = 30.0f;   //ゲームの制限時間
    float startCount = 4.0f;    //カウントダウンの時間

    float clearScore = 0.0f;    //クリア時の残り時間
    float bestScore = 0.0f;     //ゲーム全体でのベストタイム
    //========

    //===Sound
    // 再生を行っている音のidを格納する
    int playingMusic[3];
    //========
    
    //===Scene
    UINT m_scene;   //現在のシーン
    //========

    //ImGui
    bool check = false;

private:
    void ChangeSceneInit(UINT scene);
    void countTimer();

    //ImGui
    void imgui();

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

    int getHeartItem()
    {
        return heartItem;
    }

    void plusHeartItemCount()
    {
        heartItem++;
    }
};

