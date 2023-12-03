#pragma once
#include "UIRenderBase.h"

class GamePrograming3UIRender :
    public UIRenderBase
{
private:
    int heartItemCount = 0;

    float timerCount = 30.0f;
    float startCount = 4.0f;

    char timerText[64] = "Time:30";
    char itemText[64] = "";
    char countText[64] = "";

public:
    // GameComponent を介して継承されました
    virtual void initAction() override;
    virtual bool frameAction() override;
    virtual void finishAction() override;

    void plusHeartItemCount();
};

