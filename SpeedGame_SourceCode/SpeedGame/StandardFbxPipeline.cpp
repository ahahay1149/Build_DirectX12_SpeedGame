#include "StandardFbxPipeline.h"
#include "DXSampleHelper.h" //ThrowIfFailed等
#include "d3dx12.h"
#include "FBXCharacterData.h"

#include <MyAccessHub.h>
//========Camera Change Phase 1
#include "GamePrograming3Scene.h"
//========Camera Change Phase 1 End

HRESULT StandardFbxPipeline::InitPipeLineStateObject(ID3D12Device2* d3dDev)
{
    HRESULT hr = E_FAIL;

    //まずはバージョン設定用構造体
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

    // バージョンテスト
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

    //1.1をサポートしているかチェック
    if (FAILED(d3dDev->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;    //失敗した場合は最大バージョンを1.0に
    }

    //01: テクスチャ用RANGEの作成
    CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
    ZeroMemory(ranges, sizeof(CD3DX12_DESCRIPTOR_RANGE1));

    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC); //Texture

    //SamplerStateの設定。RootSignatureにここらへんの設定が組み込まれる
    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;	//サンプラは切り替え可能
    sampler.MipLODBias = 0;
    sampler.MaxAnisotropy = 0;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler.MinLOD = 0.0f;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister = 0;
    sampler.RegisterSpace = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    //Samplerの入れ替えが発生する場合はSamplerStateViewHeapが必要になる

    //RootSignature設定本体、D3D12_VERSIONED_ROOT_SIGNATURE_DESCを設定しやすくしたd3dx12のサポート構造体で宣言
    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc = {};

    //SkinAnime 01
    CD3DX12_ROOT_PARAMETER1 rootParameters[5];  //ボーンデータが増えるので5つに
    if (m_animationMode)
    {
        ZeroMemory(rootParameters, sizeof(CD3DX12_ROOT_PARAMETER1) * 5);
        rootParameters[0].InitAsConstantBufferView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);
        rootParameters[1].InitAsConstantBufferView(3, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);   //増えたのはこれ
        rootParameters[2].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);
        rootParameters[3].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);
        rootParameters[4].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

        //パラメータ数決め打ち、決め打ちしないとReleaseビルドが通らないので修正
        rootSignatureDesc.Init_1_1(5, rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    }
    else
    //SkinAnime End
    {

        ZeroMemory(rootParameters, sizeof(CD3DX12_ROOT_PARAMETER1) * 4);

        // 定数バッファの設定。
        rootParameters[0].InitAsConstantBufferView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);
        rootParameters[1].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);
        rootParameters[2].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);

        // これだけ違うのはテクスチャの設定なため
        rootParameters[3].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

        // 後から追記にすると修正範囲が広かったので最初から設定
        m_animationOffset = 3;

        // rootParametersをrootSignatureDescに反映
        rootSignatureDesc.Init_1_1(4, rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        //02: ここまで
    }

    // まずはRootSignatureDescへ設定パラメータを統合して設定バイナリを作成する
    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, signature.GetAddressOf(), error.GetAddressOf()));

    // 結合して出来たバイナリでID3D12RootSignatureを作成
    ThrowIfFailed(d3dDev->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.GetAddressOf())));

    // cso (Compiled Shader Object) データ保存用
    struct
    {
        byte* data;
        uint32_t size;
    } meshShader, pixelShader;	//変数宣言も終わらせているのでこのまま使う

    //03: csoの読み込み
    //SkinAnime 02
    if (m_animationMode)
        ReadDataFromFile(L"Resources/shaders/FbxSkinAnimeVertexShader.cso", &meshShader.data, &meshShader.size);
    else
    //SkinAnime End
    ReadDataFromFile(L"Resources/shaders/FbxVertexShader.cso", &meshShader.data, &meshShader.size);

    ReadDataFromFile(L"Resources/shaders/FbxPixelShader.cso", &pixelShader.data, &pixelShader.size);
    //03: ここまで

    // インプットレイアウト作成
    D3D12_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXTURE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

        //SkinAnime 03
        { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDINDICES", 1, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        //SkinAnime End
    };

    // PipelineStateの作成
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

    //04: インプットレイアウトを登録
    //SkinAnime 04
    if (m_animationMode)
        psoDesc.InputLayout = { layout, _countof(layout) };
    else
    //SkinAnime End
        psoDesc.InputLayout = { layout, 4 };    //staticモードの時はボーン関係のデータがない
    //04: ここまで

    //05: RootSignatureとシェーダの登録
    psoDesc.pRootSignature = m_rootSignature.Get();

    // cso (Compiled Shader Object) をPipelineのDescriptorに設定。
    psoDesc.VS.BytecodeLength = meshShader.size;
    psoDesc.PS.BytecodeLength = pixelShader.size;
    psoDesc.VS.pShaderBytecode = meshShader.data;
    psoDesc.PS.pShaderBytecode = pixelShader.data;
    //05: ここまで

    //ラスタライザ設定。RasterizerStateの中身は結構多いのでCD3DX12で簡易設定
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);// CW front; cull back
    psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; //ラスタライザのマルチサンプリング時マスク値
    psoDesc.SampleDesc.Count = 1;   //マルチサンプリング適用数。1だと「使わない」

    //ブレンドステート設定。これも中身が多いのでCD3DX12で簡易設定
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);         // Opaque

    //06: 簡易設定した上で、テクスチャのアルファブレンドが動作するように手動設定
    D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc = psoDesc.BlendState.RenderTarget[0];
    defaultRenderTargetBlendDesc.BlendEnable = FALSE;	//無効にしておく。有効にするターゲットは最初の物だけ

    //AlphaBlend設定
    defaultRenderTargetBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
    defaultRenderTargetBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    defaultRenderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
    defaultRenderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
    defaultRenderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
    defaultRenderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    defaultRenderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    //AlphaBlend設定を全てのRenderTargetに反映
    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        psoDesc.BlendState.RenderTarget[i] = defaultRenderTargetBlendDesc;

    //最初のターゲットのみ有効にする
    psoDesc.BlendState.RenderTarget[0].BlendEnable = true;
    //06: ここまで

    //07: Depth Stencilの設定
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;  // この設定が実際の深度バッファとずれていると描画で落ちる
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.StencilEnable = TRUE;     // 今回ステンシル使わないんだけどフォーマットをS8にしてたので

    // ステンシルがある場合は、ここから先の設定も忘れないように。忘れると描画で落ちる
    psoDesc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    psoDesc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp = //ステンシルの比較演算設定
    { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
    psoDesc.DepthStencilState.FrontFace = defaultStencilOp;
    psoDesc.DepthStencilState.BackFace = defaultStencilOp;
    //07: ここまで

    // プリミティブトポロジの設定
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    //08:レンダーターゲットの設定
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    //08: ここまで

    //09: GraphicsPipelineState作成
    ThrowIfFailed(d3dDev->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipeLineState.GetAddressOf())));
    //09: ここまで

    //10:コマンドリスト用vector確保
    {
        m_cmdLists.resize(FRAME_COUNT);
        MyGameEngine* engine = MyAccessHub::getMyGameEngine();
        for (int i = 0; i < FRAME_COUNT; i++)
        {
            ID3D12CommandAllocator* cmdAL = engine->GetCommandAllocator(i);
            ThrowIfFailed(engine->GetDirect3DDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAL, m_pipeLineState.Get(), IID_PPV_ARGS(m_cmdLists[i].GetAddressOf())));
            m_cmdLists[i]->Close();
        }

        engine->WaitForGpu();
    }
    //10: ここまで

    hr = CreateDescriptorHeaps();

    return hr;
}

ID3D12GraphicsCommandList* StandardFbxPipeline::ExecuteRender()
{

    //描画対象が無いのでここで終了
    if (m_renderList.size() < 1) return nullptr;

    UINT strides = sizeof(FbxVertex);
    UINT offsets = 0;

    MyGameEngine* engine = MyAccessHub::getMyGameEngine();

    //========Camera Change Phase 1
    GamePrograming3Scene* scene = static_cast<GamePrograming3Scene*>(engine->GetSceneController());
    std::wstring cameraLabel = L"";  //カメラ未設定状態
    //========Camera Change Phase 1 End

    //各種マネージャの取得
    MeshManager* pMeshMng = engine->GetMeshManager();
    TextureManager* pTextureMng = engine->GetTextureManager();

    //CommandAllocator取得
    ID3D12CommandAllocator* cmdAl = engine->GetCurrentCommandAllocator();

    //エンジンで管理しているフレームのインデックス値も取得
    UINT frameIndex = engine->GetCurrentFrameIndex();

    //コマンドリスト取得
    ID3D12GraphicsCommandList* cmdList = m_cmdLists[frameIndex].Get();

    //12: CommandListの初期化
    // リセット
    ThrowIfFailed(cmdList->Reset(cmdAl, m_pipeLineState.Get()));
    //12: ここまで

    //RenderTargetとViewPortの設定
    engine->SetMainRenderTarget(cmdList);

    //13: Resource Barrierの設定
    CD3DX12_RESOURCE_BARRIER tra = 
        CD3DX12_RESOURCE_BARRIER::Transition(engine->GetRenderTarget(frameIndex), 
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    cmdList->ResourceBarrier(1, &tra);  //セット。第一引数は設定の数
    //13:ここまで

    //14: RootSignatureのセット
    cmdList->SetGraphicsRootSignature(m_rootSignature.Get());
    //14: ここまで

    //トポロジ設定
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    //15:ディスクリプタヒープのセット
    ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
    cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    //15: ここまで

    //メッシュ個別設定
    for (auto charaData : m_renderList)
    {
        FBXCharacterData* p_fbxChara = static_cast<FBXCharacterData*>(charaData);

        //========Camera Change Phase 1
        if (p_fbxChara->GetCameraLabel() != cameraLabel)
        {
            auto comp = scene->getCameraComponent(p_fbxChara->GetCameraLabel());
            if (comp != nullptr)
            {
                cameraLabel = p_fbxChara->GetCameraLabel();
                CharacterData* camChar = comp->getGameObject()->getCharacterData();
                ID3D12Resource* p_viewMtx = camChar->GetConstantBuffer(0);
                ID3D12Resource* p_prjMtx = camChar->GetConstantBuffer(1);
                if (m_animationMode)
                {
                    cmdList->SetGraphicsRootConstantBufferView(2, p_viewMtx->GetGPUVirtualAddress());//View
                    cmdList->SetGraphicsRootConstantBufferView(3, p_prjMtx->GetGPUVirtualAddress());//Projection
                }
                else
                {
                    cmdList->SetGraphicsRootConstantBufferView(1, p_viewMtx->GetGPUVirtualAddress());//View
                    cmdList->SetGraphicsRootConstantBufferView(2, p_prjMtx->GetGPUVirtualAddress());//Projection
                }
            }
        }
        //========Camera Change Phase 1 End


        //FbxCharacterDataのメソッドでmainFbx取り出し
        FBXDataContainer* mainFbx = p_fbxChara->GetMainFbx();

        //17: World(Model)マトリクスのセット
        ID3D12Resource* p_WorldMtx = p_fbxChara->GetConstantBuffer(0);

        cmdList->SetGraphicsRootConstantBufferView(0, p_WorldMtx->GetGPUVirtualAddress());//World
        
        XMMATRIX storeMatrix = XMMatrixTranspose(p_fbxChara->GetWorldMatrix());
        //=========移動する地形　対応 END

        // GPUにアップロード
        engine->UpdateShaderResourceOnGPU(p_WorldMtx, &storeMatrix, sizeof(XMMATRIX));
        //17: ここまで

        MeshContainer* mesh = nullptr;

        //SkinAnime 07
        if (m_animationMode)
        {
            // スキン有り ボーンデータ更新
            ID3D12Resource* resource = p_fbxChara->GetConstantBuffer(mainFbx->GetCBuffIndex());
            cmdList->SetGraphicsRootConstantBufferView(1, resource->GetGPUVirtualAddress());//Skin
            engine->UpdateShaderResourceOnGPU(resource, p_fbxChara->GetAnimatedMatrixData(), sizeof(XMFLOAT4X4) * mainFbx->GetClusterCount());
        }
        //SkinAnime End

        //MainFbxに格納された分割メッシュを全てレンダリング
        for (int meshIndex = 0; (mesh = mainFbx->GetMeshContainer(meshIndex)) != nullptr; meshIndex++)
        {
            //18: 頂点データ・インデックスデータ設定。
            pMeshMng->SetVertexBuffer(cmdList, mesh->m_MeshId);
            pMeshMng->SetIndexBuffer(cmdList, mesh->m_MeshId);
            //18: ここまで

            // テクスチャ設定
            // マテリアル名が空ならテクスチャもないためスルー
            if (mesh->m_MaterialId != L"")
            {
                //MainFbxのマテリアルコンテナを取り出し
                MaterialContainer* matCon = mainFbx->GetMaterialContainer(mesh->m_MaterialId);

                //マテリアルコンテナ内のDiffuseTextureの数を取得。
                int textureLength = matCon->m_diffuseTextures.size();
                
                //19: 取得したテクスチャをシェーダにセット
                auto gpuHeap = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
                // 構造体名が長いのでautoで記述

                for (int i = 0; i < textureLength; i++)
                {
                    //テクスチャマネージャ用のラベル名を取得
                    std::wstring texId = matCon->m_diffuseTextures[i];

                    //このパイプラインに登録したテクスチャリストにラベルが登録されているか確認
                    if (m_srvTexList.find(texId) == m_srvTexList.end())
                    {
                        pTextureMng->CreateTextureSRV(engine->GetDirect3DDevice(), m_srvHeap.Get(),
                            m_animationOffset + m_numOfTex, texId);
                        //パイプライン用テクスチャリストにラベル名と登録したインデックス番号を設定
                        m_srvTexList[texId] = m_numOfTex;
                        //登録用インデックス番号を進める
                        m_numOfTex++;
                    }

                    gpuHeap.ptr += 
                        engine->GetDirect3DDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
                        * (m_animationOffset + m_srvTexList[texId]);

                    CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle(gpuHeap);
                    cmdList->SetGraphicsRootDescriptorTable(m_animationOffset + i, cbvSrvGpuHandle);
                }
                //19: ここまで
            }

            //20:描画
            cmdList->DrawIndexedInstanced(
                (UINT)mesh->m_indexData.size(),
                1, 0, 0, 0);
            //20: ここまで
        }

    }

    //21: リソースバリア解除
    tra = CD3DX12_RESOURCE_BARRIER::Transition(engine->GetRenderTarget(frameIndex), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    // リソースバリアコマンドセット
    cmdList->ResourceBarrier(1, &tra);
    //21: ここまで

    m_renderList.clear();
    cmdList->Close();

    return cmdList;
}

HRESULT StandardFbxPipeline::CreateDescriptorHeaps()
{
    ID3D12Device* d3dDev = MyAccessHub::getMyGameEngine()->GetDirect3DDevice();


    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};

    cbvHeapDesc.NumDescriptors = 20; //最大テクスチャ数

    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    return d3dDev->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(m_srvHeap.ReleaseAndGetAddressOf()));
}
