#pragma once
#include <GameObject.h>
class MovingPlatform :
    public GameComponent
{
private:
    int sta;
    float hRad;
    float rotDeg;
    float yPos;

public:
    virtual void hitReaction(GameObject* targetGo, HitAreaBase* hit) override;

    // GameComponent を介して継承されました
    virtual void initAction() override;
    virtual bool frameAction() override;
    virtual void finishAction() override;
};

