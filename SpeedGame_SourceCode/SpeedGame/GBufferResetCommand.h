#pragma once
#include "PipeLineManager.h"
class GBufferResetCommand :
    public GraphicsPipeLineObjectBase
{
    // GraphicsPipeLineObjectBase を介して継承されました
    HRESULT CreateDescriptorHeaps() override;
    HRESULT InitPipeLineStateObject(ID3D12Device2* d3dDev) override;
    ID3D12GraphicsCommandList* ExecuteRender() override;
};

