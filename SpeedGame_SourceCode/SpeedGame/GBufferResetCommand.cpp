#include "GBufferResetCommand.h"
#include "DXSampleHelper.h"

HRESULT GBufferResetCommand::CreateDescriptorHeaps()
{
    return S_OK;    //Decriptor Heapなし
}

HRESULT GBufferResetCommand::InitPipeLineStateObject(ID3D12Device2* d3dDev)
{
    m_cmdLists.resize(FRAME_COUNT);
    MyGameEngine* engine = MyAccessHub::getMyGameEngine();
    for (int i = 0; i < FRAME_COUNT; i++)
    {
        // CommandAllocator分のCommandListを作成
        ID3D12CommandAllocator* cmdAL = engine->GetCommandAllocator(i);
        ThrowIfFailed(engine->GetDirect3DDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAL, m_pipeLineState.Get(), IID_PPV_ARGS(m_cmdLists[i].GetAddressOf())));
        m_cmdLists[i]->Close();
    }

    return S_OK;
}

ID3D12GraphicsCommandList* GBufferResetCommand::ExecuteRender()
{
    MyGameEngine* myEngine = MyAccessHub::getMyGameEngine();
    MeshManager* mshMng = myEngine->GetMeshManager();

    ID3D12CommandAllocator* cmdAl = myEngine->GetCurrentCommandAllocator();

    UINT frameIndex = myEngine->GetCurrentFrameIndex();

    ID3D12GraphicsCommandList* cmdList = m_cmdLists[frameIndex].Get();

    ThrowIfFailed(cmdList->Reset(cmdAl, m_pipeLineState.Get()));

    CD3DX12_RESOURCE_BARRIER tra[1];

    //================Normal Buffer=================
    auto texBuff = myEngine->GetTextureManager()->GetTexture(L"NormalBuffer");
    ID3D12Resource* nmlBuff = texBuff->m_pTexture.Get();

    //NormalバッファをRENDER TARGETに切り替え
    tra[0] = CD3DX12_RESOURCE_BARRIER::Transition(nmlBuff,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

    cmdList->ResourceBarrier(1, tra);  //セット。第一引数は設定の数

    // 法線バッファのクリア
    cmdList->ClearRenderTargetView(texBuff->descHeap->GetCPUDescriptorHandleForHeapStart(), Colors::Transparent, 0, nullptr);

    //バリアを戻す
    tra[0] = CD3DX12_RESOURCE_BARRIER::Transition(nmlBuff,
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    cmdList->ResourceBarrier(1, tra);  //セット。第一引数は設定の数
    //================Normal Buffer End=============

    //======Depth Shadow
    // シャドウマップのDSV取得
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(myEngine->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart());
    dsvHandle.ptr += myEngine->GetDirect3DDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    D3D12_RECT scissor;
    scissor.left = 0;
    scissor.top = 0;
    scissor.right = myEngine->GetLightDepthTextureSize().x;
    scissor.bottom = myEngine->GetLightDepthTextureSize().y;
    cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 1, &scissor);
    //======Depth Shadow End

    cmdList->Close();

    return cmdList;
}
