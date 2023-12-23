#pragma once
#include "PipeLineManager.h"

class EdgeDrawPipeline :
    public GraphicsPipeLineObjectBase
{
private:
    ComPtr<ID3D12DescriptorHeap> m_srvHeap;

    // GraphicsPipeLineObjectBase を介して継承されました
    HRESULT CreateDescriptorHeaps() override;

public:
    HRESULT InitPipeLineStateObject(ID3D12Device2* d3dDev) override;
    ID3D12GraphicsCommandList* ExecuteRender() override;
};

