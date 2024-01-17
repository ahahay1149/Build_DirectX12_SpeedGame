#pragma once
#include "UIRenderBase.h"

class GameOverUIRender :
    public UIRenderBase
{
private:
    char retryText[32] = "Restart/ R key";
    char titleText[32] = "Title/ Space Key";

public:

    // GameComponent を介して継承されました
    void initAction() override;
    bool frameAction() override;
    void finishAction() override;
};