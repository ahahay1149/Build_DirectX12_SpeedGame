#pragma once
#include "PipeLineManager.h"

class ShadowMapPipeline :
    public GraphicsPipeLineObjectBase
{
private:
    bool m_staticMesh = false;
    XMMATRIX m_lightProjMtx;
    ComPtr<ID3D12Resource> m_cbLightProjMtx;

    // GraphicsPipeLineObjectBase を介して継承されました
    HRESULT CreateDescriptorHeaps() override;

public:
    D3D12_VIEWPORT m_viewport;  //ビューポート
    D3D12_RECT m_scissorRect;   //シザーレクト

    void SetStaticMeshMode(bool flg)
    {
        m_staticMesh = flg;
    }

    XMMATRIX GetLightProjectionMatrix()
    {
        return m_lightProjMtx;
    }

    HRESULT InitPipeLineStateObject(ID3D12Device2* d3dDev) override;
    ID3D12GraphicsCommandList* ExecuteRender() override;
};
