#pragma once
#include "UIRenderBase.h"

class ResultUIRender :
    public UIRenderBase
{
private:
    float clearCount = 0;
    float bestScore = 0;

    char clearCountText[64] = "";
    char bestScoreText[64] = "";
    char scoreUpdateText[64];
    int scoreUpdateTimer;
    char retryText[32] = "Restart/ Mouse Click";
    char titleText[32] = "Title/ Space Key";

public:

    // GameComponent を介して継承されました
    virtual void initAction() override;
    virtual bool frameAction() override;
    virtual void finishAction() override;
};

