﻿#include "StandardLightingPipeline.h"
#include "DXSampleHelper.h"
#include "d3dx12.h"

#include <MyAccessHub.h>
#include "GamePrograming3Scene.h"

#include "LightSettingManager.h"

int NullFunction(int parameterIndex, std::wstring& currentLabel, MyGameEngine* engine, ID3D12GraphicsCommandList* cmdList, FBXCharacterData* fbxChara)
{
    return ++parameterIndex;    //RootSignatureのIndexを一つ進める
}

int CmdBoneSet(int parameterIndex, std::wstring& currentLabel, MyGameEngine* engine, ID3D12GraphicsCommandList* cmdList, FBXCharacterData* fbxChara)
{
    // スキン有り ボーンデータ更新
    auto mainFbx = fbxChara->GetMainFbx();
    ID3D12Resource* resource = fbxChara->GetConstantBuffer(mainFbx->GetCBuffIndex());
    cmdList->SetGraphicsRootConstantBufferView(parameterIndex, resource->GetGPUVirtualAddress());//Skin

    ++parameterIndex;   //インデックスを進める

    return parameterIndex;
}

int CmdAmbientLight(int parameterIndex, std::wstring& currentLabel, MyGameEngine* engine, ID3D12GraphicsCommandList* cmdList, FBXCharacterData* fbxChara)
{
	// 01: 環境光データをパイプラインにセット
    if (currentLabel != fbxChara->GetAmbientLight())    //引数のラベルとfbxCharaのラベルが異なる=新しい設定
    {
        currentLabel = fbxChara->GetAmbientLight();     //ラベルを入れ替え
        auto lightMng = LightSettingManager::GetInstance();
        ID3D12Resource* resource = lightMng->GetAmbientLight(currentLabel)->GetConstantBuffer();        //AmbientLight
        cmdList->SetGraphicsRootConstantBufferView(parameterIndex, resource->GetGPUVirtualAddress());
    }
	// 01: ここまで

    ++parameterIndex;   //インデックスを進める
    return parameterIndex;
}

int CmdDirectionalLight(int parameterIndex, std::wstring& currentLabel, MyGameEngine* engine, ID3D12GraphicsCommandList* cmdList, FBXCharacterData* fbxChara)
{
	// 02: 平行光源データをパイプラインにセット
    if (currentLabel != fbxChara->GetDirectionalLight())
    {
        currentLabel = fbxChara->GetDirectionalLight();
        auto lightMng = LightSettingManager::GetInstance();
        ID3D12Resource* resource = lightMng->GetDirectionalLight(currentLabel)->GetConstantBuffer();    //DirectionalLight
        cmdList->SetGraphicsRootConstantBufferView(parameterIndex, resource->GetGPUVirtualAddress());
    }
	// 02: ここまで

    ++parameterIndex;   //インデックスを進める
    return parameterIndex;
}

void StandardLightingPipeline::SetTextureToCommandLine(MyGameEngine* engine, TextureManager* pTextureMng, ID3D12GraphicsCommandList* cmdList, int prmIndex, std::wstring texId)
{
    // このパイプラインに登録したテクスチャリストにラベルが登録されているか確認
    if (m_srvTexList.find(texId) == m_srvTexList.end())
    {
        //登録されていない場合SRV用ヒープにテクスチャのVIEWを作成
        pTextureMng->CreateTextureSRV(engine->GetDirect3DDevice(), m_srvHeap.Get(),
            m_numOfTex, texId);
        //パイプライン用テクスチャリストにラベル名と登録したインデックス番号を設定
        m_srvTexList[texId] = m_numOfTex;
        //登録用インデックス番号を進める
        m_numOfTex++;
    }

    //構造体のアドレス値をセットするテクスチャのVIEWを保存した位置に
    auto gpuHeap = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
    gpuHeap.ptr +=
        engine->GetDirect3DDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
        * (m_srvTexList[texId]);

    //CommandListを通してRootSignatureとGPUアドレスを結びつける
    CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle(gpuHeap);

    cmdList->SetGraphicsRootDescriptorTable(prmIndex, cbvSrvGpuHandle);
}

void StandardLightingPipeline::SetPipelineFlags(UINT32 flg)
{
    m_pipelineFlg = flg;

    if (flg & PIPELINE_FLAGS::SKELTAL)
    {
        m_BoneFunc = &CmdBoneSet;
    }
    else
    {
        m_BoneFunc = &NullFunction;
    }

	// 03: 関数ポインタによる設定自動分岐
    if (flg & PIPELINE_FLAGS::Lambert)
    {
        m_AmbientFunc = &CmdAmbientLight;
        m_DirectionalFunc = &CmdDirectionalLight;
    }
    else
    {
        m_AmbientFunc = &NullFunction;
        m_DirectionalFunc = &NullFunction;
    }
	// 03: ここまで

}

HRESULT StandardLightingPipeline::InitPipeLineStateObject(ID3D12Device2* d3dDev)
{
    HRESULT hr = E_FAIL;

    // バージョン設定用構造体
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

    // バージョンテスト
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

    //1.1をサポートしているかチェック
    if (FAILED(d3dDev->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;    //失敗した場合は最大バージョンを1.0に
    }

    // テクスチャ用RANGEの作成

    CD3DX12_DESCRIPTOR_RANGE1 ranges[5];
    ZeroMemory(ranges, sizeof(CD3DX12_DESCRIPTOR_RANGE1) * 5);

    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC); //Texture

    //======Specular + Normal
    ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC); //Specular Map
    ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC); //Normal Map
    //======Specular + Normal End
    //======Toon Shader
    ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC); //ColorTone Texture
    //======Toon Shader End
    //======Depth Shadow
    ranges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);         //Shadow Map Texture
    //======Depth Shadow End

    //SamplerStateの設定
    D3D12_STATIC_SAMPLER_DESC sampler[2] = {};

    //Shadow Bler
    sampler[0].Filter = D3D12_FILTER_ANISOTROPIC;
    sampler[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler[0].MipLODBias = 0;
    sampler[0].MaxAnisotropy = 4;
    sampler[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler[0].MinLOD = 0.0f;
    sampler[0].MaxLOD = D3D12_FLOAT32_MAX;
    sampler[0].ShaderRegister = 0;
    sampler[0].RegisterSpace = 0;
    sampler[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    sampler[1].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; //比較+線形補完フィルタ
    sampler[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler[1].MipLODBias = 0;
    sampler[1].MaxAnisotropy = 1;
    sampler[1].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    sampler[1].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler[1].MinLOD = 0.0f;
    sampler[1].MaxLOD = D3D12_FLOAT32_MAX;
    sampler[1].ShaderRegister = 1;  //S1で登録するので1
    sampler[1].RegisterSpace = 0;
    sampler[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    //Shadow Bler End

    //RootSignature設定本体
    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc = {};

    CD3DX12_ROOT_PARAMETER1 rootParameters[14];  //基本MVPマトリクス 3 テクスチャ 5 + マテリアル設定 1 + ボーン 1 + 環境光 1 + ライトVP 1 + 平行光源 1 + カメラ光源 1
    ZeroMemory(rootParameters, sizeof(CD3DX12_ROOT_PARAMETER1) * 14);

    //増える可能性あるので変数化
    int paramIndex = 0;
    m_textureIndex = m_worldMtxIndex = m_lightIndex = m_toneIndex = m_shadowDepthIndex -1;

    int texCount = 0;  //テクスチャSRVを保存するGPU内インデックス番号

    //======Specular + Normal
    // マテリアル設定 スロット番号6
    rootParameters[paramIndex++].InitAsConstantBufferView(6, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
    //======Specular + Normal End

    m_textureIndex = paramIndex;
    rootParameters[paramIndex++].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);   //Diffuse テクスチャ
    texCount++;    //テクスチャの数を保存

    //======Specular + Normal
    rootParameters[paramIndex++].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);   //Specular テクスチャ
    texCount++;
    rootParameters[paramIndex++].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL);   //Normal テクスチャ
    texCount++;
    //======Specular + Normal End

    m_worldMtxIndex = paramIndex;
    rootParameters[paramIndex++].InitAsConstantBufferView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL); //Model(World)

	// Staticメッシュだと使わないけどRootSignatureには登録しておく
    rootParameters[paramIndex++].InitAsConstantBufferView(4, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);   //ボーン用

    rootParameters[paramIndex++].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);   //Projection
    rootParameters[paramIndex++].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);   //View

    //======Specular
    rootParameters[paramIndex++].InitAsConstantBufferView(5, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
    //======Specular End

    m_lightIndex = paramIndex;

	// 04: RootSignatureに平行光源と環境光の定数バッファ設定を登録
    rootParameters[paramIndex++].InitAsConstantBufferView(3, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
        D3D12_SHADER_VISIBILITY_ALL);   //平行光源
    rootParameters[paramIndex++].InitAsConstantBufferView(4, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
        D3D12_SHADER_VISIBILITY_PIXEL); //環境光 ボーンとレジスタ被らせてる
	// 04: ここまで

    //======Depth Shadow
    m_shadowDepthIndex = paramIndex;
    rootParameters[paramIndex++].InitAsDescriptorTable(1, &ranges[4], D3D12_SHADER_VISIBILITY_ALL); //ShadowMapTexture
    texCount++;

    rootParameters[paramIndex++].InitAsConstantBufferView(7, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);   //ライトVP
    //======Depth Shadow End

    //======Toon Shader
    m_toneIndex = paramIndex;
    rootParameters[paramIndex++].InitAsDescriptorTable(1, &ranges[3], D3D12_SHADER_VISIBILITY_PIXEL);
    texCount++;
    //======Toon Shader End

    rootSignatureDesc.Init_1_1(paramIndex, rootParameters, 2, sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
 
    // RootSignatureの作成
    // RootSignatureDescへ設定パラメータを統合して設定バイナリを作成
    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, signature.GetAddressOf(), error.GetAddressOf()));

    // 結合して出来たバイナリでID3D12RootSignatureを作成
    ThrowIfFailed(d3dDev->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.GetAddressOf())));

    //======Depth Shadow
    if ((m_pipelineFlg & PIPELINE_FLAGS::LIGHTING_MASK) != 0)
    {
        if ((m_pipelineFlg & PIPELINE_FLAGS::SKELTAL) != 0)
        {
            m_shadowMapPL = static_cast<ShadowMapPipeline*>(MyAccessHub::getMyGameEngine()->
                GetPreDrawPipelineManager()->GetPipeLineObject(L"SkeltalShadowMap"));
        }
        else
        {
            m_shadowMapPL = static_cast<ShadowMapPipeline*>(MyAccessHub::getMyGameEngine()->
                GetPreDrawPipelineManager()->GetPipeLineObject(L"StaticShadowMap"));
        }
    }
    //======Depth Shadow End

    // cso (Compiled Shader Object) データ保存用
    struct
    {
        byte* data;
        uint32_t size;
    } meshShader, pixelShader;

    // csoの読み込み
    if (m_pipelineFlg & PIPELINE_FLAGS::LIGHTING_MASK)
    {
		// 05: シェーダ登録処理
        if (m_pipelineFlg & PIPELINE_FLAGS::SKELTAL)
            ReadDataFromFile(L"Resources/shaders/SkeltalMeshVertexShader.cso", &meshShader.data, &meshShader.size);
        else
            ReadDataFromFile(L"Resources/shaders/StaticMeshVertexShader.cso", &meshShader.data, &meshShader.size);

        //===Toon
        if ((m_pipelineFlg & PIPELINE_FLAGS::Toon) == PIPELINE_FLAGS::Toon)
        {
            ReadDataFromFile(L"Resources/shaders/ToonShader.cso", &pixelShader.data, &pixelShader.size);
        }
        //===Phong
        else if (m_pipelineFlg & PIPELINE_FLAGS::Phong)
        {
            ReadDataFromFile(L"Resources/shaders/PhongShader.cso", &pixelShader.data, &pixelShader.size);
        }
        //===Blinn Phong
        else if (m_pipelineFlg & PIPELINE_FLAGS::Blinn)
        {
            ReadDataFromFile(L"Resources/shaders/BlinnPhongShader.cso", &pixelShader.data, &pixelShader.size);
        }
        //===Other
        else
        {
            ReadDataFromFile(L"Resources/shaders/LightingPixelShader.cso", &pixelShader.data, &pixelShader.size);
        }

		// 05: ここまで
    }
    else
    {
        if (m_pipelineFlg & PIPELINE_FLAGS::SKELTAL)
            ReadDataFromFile(L"Resources/shaders/FbxSkinAnimeVertexShader.cso", &meshShader.data, &meshShader.size);
        else
            ReadDataFromFile(L"Resources/shaders/FbxVertexShader.cso", &meshShader.data, &meshShader.size);

        ReadDataFromFile(L"Resources/shaders/FbxPixelShader.cso", &pixelShader.data, &pixelShader.size);
    }
 
    // インプットレイアウト作成
    D3D12_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0 ,24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

        { "TEXTURE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

        { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDINDICES", 1, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // PipelineStateの作成
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

    // インプットレイアウト登録
    if (m_pipelineFlg & PIPELINE_FLAGS::SKELTAL)
        psoDesc.InputLayout = { layout, _countof(layout) };
    else
        psoDesc.InputLayout = { layout, 5 };

    // RootSignatureとシェーダの登録
    psoDesc.pRootSignature = m_rootSignature.Get();

    psoDesc.VS.BytecodeLength = meshShader.size;
    psoDesc.PS.BytecodeLength = pixelShader.size;
    psoDesc.VS.pShaderBytecode = meshShader.data;
    psoDesc.PS.pShaderBytecode = pixelShader.data;

    //ラスタライザ設定
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    psoDesc.SampleDesc.Count = 1;   //マルチサンプリングを適用しない

    //ブレンドステート設定
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

    D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc = psoDesc.BlendState.RenderTarget[0];
    defaultRenderTargetBlendDesc.BlendEnable = FALSE;	//無効にしておく

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

    //最初のターゲットのみ有効
    psoDesc.BlendState.RenderTarget[0].BlendEnable = true;

    //======Toon Shader
    if ((m_pipelineFlg & PIPELINE_FLAGS::Toon) == PIPELINE_FLAGS::Toon)
        psoDesc.BlendState.RenderTarget[1].BlendEnable = true;  //法線保存用
    //======Toon Shader End

    // Depth Stencilの設定
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.StencilEnable = TRUE;

    // ステンシルがある場合はここから先の設定も行う
    psoDesc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    psoDesc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
    { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
    psoDesc.DepthStencilState.FrontFace = defaultStencilOp;
    psoDesc.DepthStencilState.BackFace = defaultStencilOp;

    // プリミティブトポロジの設定
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    //======Normal Buff
    if ((m_pipelineFlg & PIPELINE_FLAGS::Toon) == PIPELINE_FLAGS::Toon)
    {
        //メインと法線保存用の2つ
        psoDesc.NumRenderTargets = 2;
        //必要なのは2つだけなので2-7は設定しない
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.RTVFormats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;

        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = 1;
        rtvHeapDesc.NodeMask = 0;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(d3dDev->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_rtvHeap.GetAddressOf())));
        
        //======Post Effect
        m_postEffect = MyAccessHub::getMyGameEngine()->GetPostEffectPipelineManager()->GetPipeLineObject(L"EdgeDraw");
        //======Post Effect End
    }
    //======Normal Buff End
    else
    {
        // ターゲットは今回一つのみ
        psoDesc.NumRenderTargets = 1;

        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    }

    // GraphicsPipelineState作成
    ThrowIfFailed(d3dDev->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_pipeLineState.GetAddressOf())));

    // コマンドリスト用vector確保
    m_cmdLists.resize(FRAME_COUNT);
    MyGameEngine* engine = MyAccessHub::getMyGameEngine();
    for (int i = 0; i < FRAME_COUNT; i++)
    {
        // CommandAllocator分のCommandListを作成
        ID3D12CommandAllocator* cmdAL = engine->GetCommandAllocator(i);
        ThrowIfFailed(engine->GetDirect3DDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAL, m_pipeLineState.Get(), IID_PPV_ARGS(m_cmdLists[i].GetAddressOf())));
        m_cmdLists[i]->Close();
    }

    //CommandListのCloseが発生しているのでGPU待ち。ここは中身がないので無くても構わないが一応
    engine->WaitForGpu();

    hr = CreateDescriptorHeaps();

    return hr;
}

ID3D12GraphicsCommandList* StandardLightingPipeline::ExecuteRender()
{
    // 描画対象が無いのでここで終了
    if (m_renderList.size() < 1) return nullptr;

    UINT strides = sizeof(FbxVertex);
    UINT offsets = 0;

    MyGameEngine* engine = MyAccessHub::getMyGameEngine();

    GamePrograming3Scene* scene = static_cast<GamePrograming3Scene*>(engine->GetSceneController());
    std::wstring cameraLabel = L"";         //カメラ未設定状態
    std::wstring materialLabel = L"";       //マテリアル未設定状態

    // Light
    std::wstring currentAmbient = L"";      //環境光未設定
    std::wstring currentDirectional = L"";  //平行光源未設定

    std::wstring currentBone = L"";         //ボーンラベル

    MeshManager* pMeshMng = engine->GetMeshManager();
    TextureManager* pTextureMng = engine->GetTextureManager();

    // CommandAllocator取得
    ID3D12CommandAllocator* cmdAl = engine->GetCurrentCommandAllocator();

    // エンジンで管理しているフレームのインデックス値も取得
    UINT frameIndex = engine->GetCurrentFrameIndex();

    // コマンドリスト取得
    ID3D12GraphicsCommandList* cmdList = m_cmdLists[frameIndex].Get();

    // CommandListの初期化
    ThrowIfFailed(cmdList->Reset(cmdAl, m_pipeLineState.Get()));
    // この時コマンドリスト内にQueueで実行中のコマンドが残っているとエラー

    CD3DX12_RESOURCE_BARRIER tra[3];

    tra[0] =
        CD3DX12_RESOURCE_BARRIER::Transition(engine->GetRenderTarget(frameIndex),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

    //======Depth Shadow
    tra[1] =
        CD3DX12_RESOURCE_BARRIER::Transition(engine->GetLightDepth(),
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    //======Depth Shadow End

    //======Edge Draw
    if ((m_pipelineFlg & PIPELINE_FLAGS::Toon) == PIPELINE_FLAGS::Toon)
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle[2];
        CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(engine->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart());

        rtvHandle[0] = { engine->GetRTVHeap()->GetCPUDescriptorHandleForHeapStart(),
                            (int)frameIndex, engine->GetRTVDescSize() };

        rtvHandle[1] = { pTextureMng->GetTexture(L"NormalBuffer")->descHeap->GetCPUDescriptorHandleForHeapStart(),
                            0, engine->GetRTVDescSize() };  //法線バッファをRTVHeapに登録

        cmdList->OMSetRenderTargets(2, rtvHandle, FALSE, &dsvHandle);   //2つのRTVとDSV
        engine->SetDefaultViewportAndRect(cmdList);

        //Resource Barrierの設定
        tra[2] = CD3DX12_RESOURCE_BARRIER::Transition(pTextureMng->GetTexture(L"NormalBuffer")->m_pTexture.Get(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_RENDER_TARGET);

        cmdList->ResourceBarrier(3, tra);   //第一引数は設定の数
    }
    else
    {
        cmdList->ResourceBarrier(2, tra);   //第一引数は設定の数
        //RenderTargetとViewPortの設定
        engine->SetMainRenderTarget(cmdList);
    }
    //======Edge Draw End

    // RootSignatureのセット
    cmdList->SetGraphicsRootSignature(m_rootSignature.Get());

    // トポロジ
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // ディスクリプタヒープ
    ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
    cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    //======Toon Shader
    if (m_toneIndex > -1)
    {
        //ToneShaderに対応しているパイプラインの場合はトゥーンテクスチャを設定
        SetTextureToCommandLine(engine, pTextureMng, cmdList, m_toneIndex, L"ToneTexture");
    }
    //======Toon Shader End

    //======Depth Shadow
    if (m_shadowMapPL != nullptr)
    {
        //Shadow Mapは共有なので最初にセット
        auto gpuHeap = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
        gpuHeap.ptr += engine->GetDirect3DDevice()->
            GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * m_srvTexList[L"SHADOW_MAP"];

        CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle(gpuHeap);
        cmdList->SetGraphicsRootDescriptorTable(m_shadowDepthIndex, cbvSrvGpuHandle);

        auto dLight = LightSettingManager::GetInstance()->GetDirectionalLight(L"SCENE_DIRECTIONAL");

        //light ProjectionをライトのViewとShadowMapが持っているProjectionをかけて計算
        XMMATRIX lightVP = dLight->GetLightViewMtx() * m_shadowMapPL->GetLightProjectionMatrix();
        XMMATRIX trLVP = XMMatrixTranspose(lightVP); //転置
        engine->UpdateShaderResourceOnGPU(m_cbLightViewProjection.Get(), &trLVP, sizeof(XMMATRIX));

        //Light VP セット
        cmdList->SetGraphicsRootConstantBufferView(m_shadowDepthIndex + 1, m_cbLightViewProjection->GetGPUVirtualAddress());
    }
    //======Depth Shadow End

    // メッシュ個別設定
    for (auto charaData : m_renderList)
    {
        FBXCharacterData* p_fbxChara = static_cast<FBXCharacterData*>(charaData);

        int prmIndex = m_worldMtxIndex;   //RootParameterインデックス番号
        materialLabel = L"";              //マテリアルラベル初期化

        // World(Model)マトリクスのセット
        ID3D12Resource* p_WorldMtx = p_fbxChara->GetConstantBuffer(0);

        cmdList->SetGraphicsRootConstantBufferView(prmIndex, p_WorldMtx->GetGPUVirtualAddress());//World
        prmIndex++;

        XMMATRIX storeMatrix = XMMatrixTranspose(p_fbxChara->GetWorldMatrix());

        // 設定分岐 スケルタルアニメ
        prmIndex = m_BoneFunc(prmIndex, currentBone, engine, cmdList, p_fbxChara);

        //========Camera
        if (p_fbxChara->GetCameraLabel() != cameraLabel)
        {
            auto comp = scene->getCameraComponent(p_fbxChara->GetCameraLabel());
            if (comp != nullptr)
            {
                cameraLabel = p_fbxChara->GetCameraLabel();
                CharacterData* camChar = comp->getGameObject()->getCharacterData();
                ID3D12Resource* p_viewMtx = camChar->GetConstantBuffer(CB_CAM_VIEW_INDEX);
                ID3D12Resource* p_prjMtx = camChar->GetConstantBuffer(CB_CAM_PROJECTION_INDEX);
                cmdList->SetGraphicsRootConstantBufferView(prmIndex, p_viewMtx->GetGPUVirtualAddress());//View
                prmIndex++;
                cmdList->SetGraphicsRootConstantBufferView(prmIndex, p_prjMtx->GetGPUVirtualAddress());//Projection
                prmIndex++;

                //======Specular
                ID3D12Resource* p_CamPos = camChar->GetConstantBuffer(CB_CAM_POSITION_INDEX);
                cmdList->SetGraphicsRootConstantBufferView(prmIndex, p_CamPos->GetGPUVirtualAddress()); //Camera Pos
                prmIndex++;
                //======Specular End
            }
        }
        //========Camera

        prmIndex = m_lightIndex;

        // 06: 設定分岐 Lambert用光源データ取得処理を分岐
        prmIndex = m_DirectionalFunc(prmIndex, currentDirectional, engine, cmdList, p_fbxChara);
        prmIndex = m_AmbientFunc(prmIndex, currentAmbient, engine, cmdList, p_fbxChara);
        // 06: ここまで

        //FbxCharacterDataのメソッドでmainFbx取り出し
        FBXDataContainer* mainFbx = p_fbxChara->GetMainFbx();
        MeshContainer* mesh = nullptr;

        //MainFbxに格納された分割メッシュを全てレンダリング
        for (int meshIndex = 0; (mesh = mainFbx->GetMeshContainer(meshIndex)) != nullptr; meshIndex++)
        {

            // 頂点・インデックスデータ
            pMeshMng->SetVertexBuffer(cmdList, mesh->m_MeshId);
            pMeshMng->SetIndexBuffer(cmdList, mesh->m_MeshId);

            // テクスチャ設定
            // マテリアル名が空ならテクスチャもない
            if (mesh->m_MaterialId != L"" && materialLabel != mesh->m_MaterialId)
            {
                materialLabel = mesh->m_MaterialId;
                MaterialContainer* matCon = mainFbx->GetMaterialContainer(mesh->m_MaterialId);

                //======Specular & Normal Map
                // MaterialInfo
                cmdList->SetGraphicsRootConstantBufferView(0, matCon->GetMaterialInfoResource()->GetGPUVirtualAddress());
                //======Specular & Normal Map End

                //Diffuse
                int texLen = matCon->m_diffuseTextures.size();
                if (texLen > 0)
                {
                    SetTextureToCommandLine(engine, pTextureMng, cmdList, m_textureIndex, matCon->m_diffuseTextures[0]);
                }

                //======Specular & Normal Map
                // テクスチャの登録順はDiffuse Specular NormalでRootSignatureに登録しているので順番は固定
                //Specular
                texLen = matCon->m_specularTextures.size();
                if (texLen > 0)
                {
                    SetTextureToCommandLine(engine, pTextureMng, cmdList, m_textureIndex + 1, matCon->m_specularTextures[0]);
                }

                texLen = matCon->m_normalTextures.size();
                if (texLen > 0)
                {
                    SetTextureToCommandLine(engine, pTextureMng, cmdList, m_textureIndex + 2, matCon->m_normalTextures[0]);
                }
                //======Specular & Normal Map End
            }

            //Drawコマンド
            cmdList->DrawIndexedInstanced((UINT)mesh->m_indexData.size(), 1, 0, 0, 0);
        }

    }

    // リソースバリア解除   
    tra[0] = CD3DX12_RESOURCE_BARRIER::Transition(engine->GetRenderTarget(frameIndex),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

    //======Depth Shadow
    tra[1] = CD3DX12_RESOURCE_BARRIER::Transition
    (engine->GetLightDepth(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_DEPTH_WRITE);
    //======Depth Shadow End

    //======Edge Draw
    if ((m_pipelineFlg & PIPELINE_FLAGS::Toon) == PIPELINE_FLAGS::Toon)
    {
        tra[2] = CD3DX12_RESOURCE_BARRIER::Transition(pTextureMng->GetTexture(L"NormalBuffer")->m_pTexture.Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        cmdList->ResourceBarrier(3, tra);
    }
    else
    {
        cmdList->ResourceBarrier(2, tra);
    }
    //======Edge Draw End

    m_renderList.clear();
    cmdList->Close();

    return cmdList;
}

void StandardLightingPipeline::AddRenerObject(CharacterData* obj)
{
    GraphicsPipeLineObjectBase::AddRenerObject(obj);    //通常処理

    //======Depth Shadow (Pre Effect)
    FBXCharacterData* fbxChara = static_cast<FBXCharacterData*>(obj);
    if (m_shadowMapPL != nullptr && fbxChara->GetShadowFlag())
    {
        //登録されるCharacterDataをそのままシャドウマップにも追加（影出力対象に）
        m_shadowMapPL->AddRenerObject(obj);
    }
    //======Depth Shadow (Pre Effect) End

    //======Post Effect
    if (m_postEffect != nullptr)
    {
        m_postEffect->AddRenerObject(obj);  //描画対象の有無だけ確認
    }
    //======Post Effect End

    MyGameEngine* engine = MyAccessHub::getMyGameEngine();

    ID3D12Resource* p_WorldMtx = obj->GetConstantBuffer(0);
    XMMATRIX storeMatrix = XMMatrixTranspose(obj->GetWorldMatrix());

    // GPUにアップロード
    engine->UpdateShaderResourceOnGPU(p_WorldMtx, &storeMatrix, sizeof(XMMATRIX));

    if ((m_pipelineFlg & PIPELINE_FLAGS::SKELTAL) != 0)
    {
        // スキン有り ボーンデータ更新
        FBXCharacterData* p_fbxChara = static_cast<FBXCharacterData*>(obj);
        auto mainFbx = p_fbxChara->GetMainFbx();
        ID3D12Resource* resource = p_fbxChara->GetConstantBuffer(mainFbx->GetCBuffIndex());
        engine->UpdateShaderResourceOnGPU(resource, p_fbxChara->GetAnimatedMatrixData(), sizeof(XMFLOAT4X4) * mainFbx->GetClusterCount());
    }

}

HRESULT StandardLightingPipeline::CreateDescriptorHeaps()
{
    MyGameEngine* engine = MyAccessHub::getMyGameEngine();
    ID3D12Device* d3dDev = engine->GetDirect3DDevice();

    //シェーダリソース用のヒープを確保
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};

    cbvHeapDesc.NumDescriptors = 20;    //最大テクスチャ数20
    cbvHeapDesc.NodeMask = 0;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    HRESULT hr = d3dDev->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(m_srvHeap.ReleaseAndGetAddressOf()));
    
    if (FAILED(hr)) return hr;

    //======Depth Shadow
    if (m_shadowMapPL != nullptr)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE heapHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        d3dDev->CreateShaderResourceView(engine->GetLightDepth(), &srvDesc, heapHandle);

        //パイプライン用テクスチャリストにラベル名と登録したインデックス番号を設定
        m_srvTexList[L"SHADOW_MAP"] = m_numOfTex;
        //登録用インデックス番号を進める
        m_numOfTex++;

        //LightのView Projection行列用
        XMMATRIX mat = {};
        engine->CreateConstantBuffer(m_cbLightViewProjection.ReleaseAndGetAddressOf(), &mat, sizeof(XMMATRIX));
    }
    //======Depth Shadow End

    return hr;
}
