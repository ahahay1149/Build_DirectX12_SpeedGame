#pragma once
#include "UIRenderBase.h"

class GamePrograming3UIRender :
    public UIRenderBase
{
private:

    char timerText[16] = "Time:30";
    char itemText[16] = "";
    char countText[16] = "";
    char playerSpeedText[16] = "";

public:
    // GameComponent を介して継承されました
    void initAction() override;
    bool frameAction() override;
    void finishAction() override;
};