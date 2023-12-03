#pragma once
#include "UIRenderBase.h"

class TitleUIRender :
    public UIRenderBase
{
private:
    char uiText[64] = "Press:Space Key";

public:

    // GameComponent を介して継承されました
    virtual void initAction() override;
    virtual bool frameAction() override;
    virtual void finishAction() override;
};