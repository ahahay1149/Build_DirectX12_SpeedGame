#pragma once
#include "UIRenderBase.h"

class ResultUIRender :
    public UIRenderBase
{
private:

    char clearCountText[64] = "";
    char bestScoreText[64] = "";
    char scoreUpdateText[64];
    int scoreUpdateTimer;
    char retryText[32] = "Restart/ R key";
    char titleText[32] = "Title/ Space Key";

public:

    // GameComponent を介して継承されました
    void initAction() override;
    bool frameAction() override;
    void finishAction() override;
};