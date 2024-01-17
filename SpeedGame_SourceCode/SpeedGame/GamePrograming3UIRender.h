#pragma once
#include "UIRenderBase.h"

class GamePrograming3UIRender :
    public UIRenderBase
{
private:

    char timerText[64] = "Time:30";
    char itemText[64] = "";
    char countText[64] = "";

public:
    // GameComponent を介して継承されました
    void initAction() override;
    bool frameAction() override;
    void finishAction() override;
};