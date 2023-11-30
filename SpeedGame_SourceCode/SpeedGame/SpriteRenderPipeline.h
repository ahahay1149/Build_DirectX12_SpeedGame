#pragma once
#include "PipeLineManager.h"

#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class SpriteRenderPipeline :
    public GraphicsPipeLineObjectBase
{
private:
    static const UINT MAX_SPRITES = 512;

    uint8_t blendMode = 0;
    uint8_t samplerMode = 0;

    //=====Color Mix
    struct MixSettings
    {
        UINT colorMix;
        UINT alphaMix;
    };
    //=====Color Mix End

    ComPtr<ID3D12DescriptorHeap> m_srvHeap;     //ShaderResourceView Heap
    ComPtr<ID3D12Resource> m_pUvBuffer = nullptr;				//UV用定数バッファ
    ComPtr<ID3D12Resource> m_pSizeBuffer = nullptr;				//サイズ用定数バッファ

    //======Transform
    ComPtr<ID3D12Resource> m_pAffineMatrix = nullptr;			//アフィン変換
    //======Transform End

    //=====Color Mix
    ComPtr<ID3D12Resource> m_pColorBuffer = nullptr;			//合成色用定数バッファ
    ComPtr<ID3D12Resource> m_pMixBuffer = nullptr;				//合成設定用定数バッファ
    //=====Color Mix End

    //====DX12 Map用バッファ
    //スタックにしたくないのでメンバとして宣言
    XMFLOAT4 m_aUv[MAX_SPRITES];
    XMFLOAT2 m_aSize[MAX_SPRITES];
    XMMATRIX m_aAffin[MAX_SPRITES];
    XMFLOAT4 m_aColor[MAX_SPRITES];
    MixSettings m_aMix[MAX_SPRITES];
    //====DX12 Map用バッファ

    // GraphicsPipeLineObjectBase を介して継承されました
public:
    virtual HRESULT InitPipeLineStateObject(ID3D12Device2* d3dDev) override;
    virtual ID3D12GraphicsCommandList* ExecuteRender() override;

    void SetBlendMode(uint8_t mode)
    {
        blendMode = mode;
    }

    void SetSamplerMode(uint8_t mode)
    {
        samplerMode = mode;
    }

    // GraphicsPipeLineObjectBase を介して継承されました
    HRESULT CreateDescriptorHeaps() override;
};

