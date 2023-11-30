﻿#pragma once
#include "GameObject.h"
#include "SpriteCharacter.h"

#include <vector>
#include <unordered_map>

#include <memory>

class GameOverUIRender :
    public GameComponent
{
private:
    //フォントデータに登録されている文字一覧。左上から順番
    const char* m_chList = "0123456789+-*/!?ABCDEFGHIJKLMNOPQRSTUVWXYZ#$%&<>abcdefghijklmnopqrstuvwxyz\"()[]|";
    const char* m_chEnd;

    std::unordered_map<char, XMFLOAT4> m_fontMap;
    std::vector<std::unique_ptr<SpriteCharacter>> m_sprites;

    int m_spriteCount;

    int MakeSpriteString(int startIndex, float ltX, float ltY, float width, float height, const char* str);

    char retryText[32] = "Restart/ Mouse Click";
    char titleText[32] = "Title/ Space Key";

public:

    // GameComponent を介して継承されました
    virtual void initAction() override;
    virtual bool frameAction() override;
    virtual void finishAction() override;
};

