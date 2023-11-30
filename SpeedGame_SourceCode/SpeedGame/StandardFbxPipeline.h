#pragma once
#include "PipeLineManager.h"
#include "FBXCharacterData.h"
class StandardFbxPipeline :
    public GraphicsPipeLineObjectBase
{
protected:

    //SkinAnime 01
    bool m_animationMode;
    //SkinAnime End

    int m_animationOffset;

    //DescriptorHeapはパイプラインごとに必要な物が変わるので派生クラス側で宣言
    ComPtr<ID3D12DescriptorHeap> m_srvHeap;

    // GraphicsPipeLineObjectBase を介して継承されました
public:
    virtual HRESULT InitPipeLineStateObject(ID3D12Device2* d3dDev) override;
    virtual ID3D12GraphicsCommandList* ExecuteRender() override;

    //SkinAnime 02
    void SetAnimationMode(bool on)
    {
        m_animationMode = on;
        if (m_animationMode)
            m_animationOffset = 4;
        else
            m_animationOffset = 3;
    }

    // GraphicsPipeLineObjectBase を介して継承されました
    HRESULT CreateDescriptorHeaps() override;
    //SkinAnime End
};
