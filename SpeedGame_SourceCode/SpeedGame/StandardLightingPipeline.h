#pragma once
#include "PipeLineManager.h"
#include "FBXCharacterData.h"

//関数ポインタ型の定義を使って処理を分岐
typedef int (*CBuffFunction)(int parameterIndex, std::wstring& currentLabel, MyGameEngine* engine, ID3D12GraphicsCommandList* cmdList, FBXCharacterData* fbxChara);

class StandardLightingPipeline :
    public GraphicsPipeLineObjectBase
{
private:
    ComPtr<ID3D12DescriptorHeap> m_srvHeap;

    UINT32  m_pipelineFlg;
    CBuffFunction m_BoneFunc;
    CBuffFunction m_AmbientFunc;
    CBuffFunction m_DirectionalFunc;

    int m_worldMtxIndex;
    int m_textureIndex;
    int m_lightIndex;

    void SetTextureToCommandLine(MyGameEngine* engine, TextureManager* pTextureMng, ID3D12GraphicsCommandList* cmdList, int prmIndex, std::wstring texId);

    // GraphicsPipeLineObjectBase を介して継承されました
    virtual HRESULT CreateDescriptorHeaps() override;

public:

    //パイプラインの動作を決めるためのビットフラグ
    enum PIPELINE_FLAGS
    {
        SKELTAL =   0x00000001,
        Lambert =   0x00000002,
        Phong   =   0x00000004,
        Blinn   =   0x00000008,

        LIGHTING_MASK = 0x0000000E,
    };

    void SetPipelineFlags(UINT32 flg);

    // GraphicsPipeLineObjectBase を介して継承されました
    virtual HRESULT InitPipeLineStateObject(ID3D12Device2* d3dDev) override;
    virtual ID3D12GraphicsCommandList* ExecuteRender() override;

    virtual void AddRenerObject(CharacterData* obj) override;
};
