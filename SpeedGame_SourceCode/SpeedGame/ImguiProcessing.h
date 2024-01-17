#pragma once
#include "ImguiEnum.h"
#include "FBXCharacterData.h"

//ImGui
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

class ImguiProcessing
{
protected:
    bool check = false;
    bool debugMode = false;

    string id;

    UINT32 debugFlg;
    GameComponent* comData;
    
    bool imguiSetting();

public:
    virtual void imgui();
    virtual void imguiInit();
    
    void setId(string Id)
    {
        id = Id;
    }

    void setDebugFlag(UINT32 flg)
    {
        debugFlg = flg;
    }

    UINT32 getDebugFlag()
    {
        return debugFlg;
    }

    void setComData(GameComponent* com)
    {
        comData = com;
    }

    GameComponent* getComData()
    {
        return comData;
    }
};