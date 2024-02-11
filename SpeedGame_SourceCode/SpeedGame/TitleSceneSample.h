#pragma once
#include "GameObject.h"
#include "SpriteCharacter.h"

#include "GamePrograming3Enum.h"

#include <memory>

class TitleSceneSample :
    public GameComponent
{
private:
    std::unique_ptr<SpriteCharacter> m_bgSp;
    std::unique_ptr<SpriteCharacter> m_messageSp;

public:
    TitleSceneSample();

    void SetBGColor(float r, float g, float b);
    void SetFontPosition(float pxY);

    // GameComponent を介して継承されました
    virtual void initAction() override;
    virtual bool frameAction() override;
    virtual void finishAction() override;
};

