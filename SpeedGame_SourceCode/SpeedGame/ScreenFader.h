#pragma once
#include "GameObject.h"
class ScreenFader :
    public GameComponent
{
private:
    int sta;

public:
    // GameComponent を介して継承されました
    virtual void initAction() override;
    virtual bool frameAction() override;
    virtual void finishAction() override;
};

