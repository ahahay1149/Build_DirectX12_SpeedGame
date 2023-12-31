#include "ShadowMapPipeline.h"

#include "FBXCharacterData.h"
#include "LightSettingManager.h"
#include "GamePrograming3Scene.h"
#include "CameraComponent.h"

#include <DXSampleHelper.h>
#include <d3dx12.h>

HRESULT ShadowMapPipeline::CreateDescriptorHeaps()
{
    return S_OK;    //Descriptor Heapなし
}

HRESULT ShadowMapPipeline::InitPipeLineStateObject(ID3D12Device2* d3dDev)
{
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

    // バージョンテスト
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

    if (FAILED(d3dDev->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)))) //1.1をサポートしているかチェック
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;    //失敗した場合は最大バージョンを1.0にする
    }

    struct
    {
        byte* data;
        uint32_t size;
    } vertexShader, pixelShader;


    //Static用とSkeltal用の2つを同時に持つ
    CD3DX12_ROOT_PARAMETER1 rootParameters[4] = {};

    //リソース用テクスチャは不要
    rootParameters[0].InitAsConstantBufferView(3, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);   //Bone
    rootParameters[1].InitAsConstantBufferView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);   //Model
    rootParameters[2].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);   //View
    rootParameters[3].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);   //Projection

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;


    // Static用とSkeltal用の2つを準備
    D3D12_INPUT_ELEMENT_DESC staticLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXTURE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_INPUT_ELEMENT_DESC skeltalLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXTURE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

        { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDINDICES", 1, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

    if (m_staticMesh)
    {
        rootSignatureDesc.Init_1_1(_countof(rootParameters) - 1, &rootParameters[1], 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, signature.GetAddressOf(), error.GetAddressOf()));
        ThrowIfFailed(d3dDev->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.GetAddressOf())));

        //頂点シェーダのみ StaticMesh
        ReadDataFromFile(L"Resources/shaders/StaticMeshShadowMapVS.cso", &vertexShader.data, &vertexShader.size);

        psoDesc.InputLayout = { staticLayout, _countof(staticLayout) };
        psoDesc.pRootSignature = m_rootSignature.Get();

        psoDesc.VS.BytecodeLength = vertexShader.size;
        psoDesc.PS.BytecodeLength = 0;
        psoDesc.VS.pShaderBytecode = vertexShader.data;
        psoDesc.PS.pShaderBytecode = nullptr;

    }
    else
    {
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, signature.GetAddressOf(), error.GetAddressOf()));
        ThrowIfFailed(d3dDev->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.GetAddressOf())));

        //頂点シェーダのみ SkeltalMesh
        ReadDataFromFile(L"Resources/shaders/SkeltalMeshShadowMapVS.cso", &vertexShader.data, &vertexShader.size);

        // Describe and create the graphics pipeline state object (PSO).
        psoDesc.InputLayout = { skeltalLayout, _countof(skeltalLayout) };
        psoDesc.pRootSignature = m_rootSignature.Get();

        psoDesc.VS.BytecodeLength = vertexShader.size;
        psoDesc.PS.BytecodeLength = 0;
        psoDesc.VS.pShaderBytecode = vertexShader.data;
        psoDesc.PS.pShaderBytecode = nullptr;

    }

    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        psoDesc.BlendState.RenderTarget[i].BlendEnable = FALSE;

    psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 0;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
    psoDesc.SampleDesc.Count = 1;   //マルチサンプリング適用数。今回は1なので使わない設定

    // Depth Stencil
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;

    // ステンシルがある場合は、ここから先の設定。忘れると描画で落ちる
    psoDesc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    psoDesc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
    { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
    psoDesc.DepthStencilState.FrontFace = defaultStencilOp;
    psoDesc.DepthStencilState.BackFace = defaultStencilOp;


    ThrowIfFailed(d3dDev->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipeLineState.GetAddressOf())));

    MyGameEngine* engine = MyAccessHub::getMyGameEngine();

    //定数バッファ作成
    m_lightProjMtx = XMMatrixOrthographicLH(engine->GetLightDepthTextureSize().x * 0.02f, engine->GetLightDepthTextureSize().y * 0.02f, 0.01f, 100.0f);
    XMMATRIX trLPMtx = XMMatrixTranspose(m_lightProjMtx);
    engine->CreateConstantBuffer(m_cbLightProjMtx.GetAddressOf(), nullptr, sizeof(XMMATRIX));

    engine->UpdateShaderResourceOnGPU(m_cbLightProjMtx.Get(), &trLPMtx, sizeof(XMMATRIX));

    //コマンドリスト用vector枠確保
    m_cmdLists.resize(FRAME_COUNT);
    for (int i = 0; i < FRAME_COUNT; i++)
    {
        // Create the command list.
        ID3D12CommandAllocator* cmdAL = engine->GetCommandAllocator(i);
        ThrowIfFailed(engine->GetDirect3DDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAL, m_pipeLineState.Get(), IID_PPV_ARGS(m_cmdLists[i].GetAddressOf())));
        m_cmdLists[i]->Close();

        NAME_D3D12_OBJECT_INDEXED(m_cmdLists, i);
    }

    engine->WaitForGpu();

    //ViewPort & Scissor Rect
    XMINT2 lightTexSize = engine->GetLightDepthTextureSize();

    m_viewport.TopLeftX = 0;
    m_viewport.TopLeftY = 0;
    m_viewport.Width = lightTexSize.x;
    m_viewport.Height = lightTexSize.y;
    m_viewport.MinDepth = 0;
    m_viewport.MaxDepth = 1.0f;

    m_scissorRect.left = 0;
    m_scissorRect.top = 0;
    m_scissorRect.right = lightTexSize.x;
    m_scissorRect.bottom = lightTexSize.y;

    return S_OK;
}

ID3D12GraphicsCommandList* ShadowMapPipeline::ExecuteRender()
{
    //描画対象が無いのでここで終了
    if (m_renderList.size() < 1) return nullptr;

    MyGameEngine* myEngine = MyAccessHub::getMyGameEngine();
    MeshManager* mshMng = myEngine->GetMeshManager();

    //コマンドラインの取得とリセット
    ID3D12CommandAllocator* cmdAl = myEngine->GetCurrentCommandAllocator();
    UINT frameIndex = myEngine->GetCurrentFrameIndex();
    ID3D12GraphicsCommandList* cmdList = m_cmdLists[frameIndex].Get();
    ThrowIfFailed(cmdList->Reset(cmdAl, m_pipeLineState.Get()));

    //DepthStencilView
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(myEngine->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart());
    dsvHandle.ptr += myEngine->GetDirect3DDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    cmdList->OMSetRenderTargets(0, nullptr, FALSE, &dsvHandle);

    cmdList->RSSetViewports(1, &m_viewport);
    cmdList->RSSetScissorRects(1, &m_scissorRect);

    cmdList->SetGraphicsRootSignature(m_rootSignature.Get());
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //Lightの位置をカメラとしてマトリクスセット
    LightSettingManager* pLightMng = LightSettingManager::GetInstance();

    auto dLight = pLightMng->GetDirectionalLight(L"SCENE_DIRECTIONAL");
    assert(dLight);

    if (m_staticMesh)
    {
        cmdList->SetGraphicsRootConstantBufferView(1, dLight->GetLightBaseMtxBuffer()->GetGPUVirtualAddress());     //View
        cmdList->SetGraphicsRootConstantBufferView(2, m_cbLightProjMtx->GetGPUVirtualAddress());                    //Projection
    }
    else
    {
        cmdList->SetGraphicsRootConstantBufferView(2, dLight->GetLightBaseMtxBuffer()->GetGPUVirtualAddress());     //View
        cmdList->SetGraphicsRootConstantBufferView(3, m_cbLightProjMtx->GetGPUVirtualAddress());                    //Projection
    }

    for (auto charaData : m_renderList)
    {
        FBXCharacterData* p_fbxChara = static_cast<FBXCharacterData*>(charaData);

        FBXDataContainer* mainFbx = p_fbxChara->GetMainFbx();

        //Modelマトリクスのセット
        ID3D12Resource* p_ModelMtx = p_fbxChara->GetConstantBuffer(0);

        XMMATRIX storeMatrix = XMMatrixTranspose(p_fbxChara->GetWorldMatrix());
        myEngine->UpdateShaderResourceOnGPU(p_ModelMtx, &storeMatrix, sizeof(XMMATRIX));

        MeshContainer* mesh = nullptr;

        if (m_staticMesh)
        {
            cmdList->SetGraphicsRootConstantBufferView(0, p_ModelMtx->GetGPUVirtualAddress());//Model
        }
        else
        {
            // スキン有り ボーンデータ更新
            ID3D12Resource* resource = p_fbxChara->GetConstantBuffer(mainFbx->GetCBuffIndex());
            cmdList->SetGraphicsRootConstantBufferView(0, resource->GetGPUVirtualAddress());//Skin
            myEngine->UpdateShaderResourceOnGPU(resource, p_fbxChara->GetAnimatedMatrixData(), sizeof(XMFLOAT4X4) * mainFbx->GetClusterCount());

            cmdList->SetGraphicsRootConstantBufferView(1, p_ModelMtx->GetGPUVirtualAddress());//Model
        }

        //MainFbxに格納された分割メッシュを全てレンダリング
        for (int meshIndex = 0; (mesh = mainFbx->GetMeshContainer(meshIndex)) != nullptr; meshIndex++)
        {
            // 頂点データ・インデックスデータ設定
            mshMng->SetVertexBuffer(cmdList, mesh->m_MeshId);
            mshMng->SetIndexBuffer(cmdList, mesh->m_MeshId);

            // 描画
            cmdList->DrawIndexedInstanced((UINT)mesh->m_indexData.size(), 1, 0, 0, 0);
        }

    }

    m_renderList.clear();
    cmdList->Close();

    return cmdList;
}
