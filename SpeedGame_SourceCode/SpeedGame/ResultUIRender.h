#pragma once
#include "UIRenderBase.h"

class ResultUIRender :
    public UIRenderBase
{
private:
    struct StageTime
    {
        char clearTime[24] = "";
        char bestTime[24] = "";
    };
    StageTime stage[int(Game::STAGE_NUM::Count) + 1];   //GameScene + Result

    char scoreUpdateText[16];
    int scoreUpdateTimer;

    char nextText[32] = "Restart/ R key";
    char titleText[32] = "Title/ Space Key";

    void displayScoreUpdate(int& count, int x, int y);

    int setX = 0;

public:

    // GameComponent を介して継承されました
    void initAction() override;
    bool frameAction() override;
    void finishAction() override;
};