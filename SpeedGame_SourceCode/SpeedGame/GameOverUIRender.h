#pragma once
#include "UIRenderBase.h"

class GameOverUIRender :
    public UIRenderBase
{
private:
    char retryText[32] = "Restart/ Mouse Click";
    char titleText[32] = "Title/ Space Key";

public:

    // GameComponent を介して継承されました
    virtual void initAction() override;
    virtual bool frameAction() override;
    virtual void finishAction() override;
};

