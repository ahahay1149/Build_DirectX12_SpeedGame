#include <Windows.h>
#include <vector>
#include "TextureManager.h"
#include <WICTextureLoader.h>	//DirectXToolKit 12から


#include "DXSampleHelper.h" //ThrowIfFailed等
#include "d3dx12.h"			//inline色々


#pragma comment(lib, "DirectXTK12.lib")
#pragma comment(lib, "DirectXTex.lib")

//======MipMap
#include <MyAccessHub.h>
//======MipMap End

//uint32_t
#include <cstdint>

using namespace std;
using namespace DirectX;


HRESULT TextureManager::InitTextureManager(void)
{
	m_textureDB.clear();

	//======MipMap
	m_mipmapGenerateList.clear();
	InitMipMapGenerator();			//MipMap生成パイプラインの生成
	//======MipMap End

	return S_OK;
}

void TextureManager::ReleaseTexObj(Texture2DContainer* txbuff)
{
	if (txbuff == nullptr) return;

	txbuff->m_scImage.Release();

	txbuff->m_pTexture.Reset();
	txbuff->m_wicData.reset();
	txbuff->m_pTextureUploadHeap.Reset();

	txbuff->descHeap.Reset();

	txbuff->m_subresouceData.pData = nullptr;

	txbuff->m_uploaded = false;
}

void TextureManager::DestructTextureManager(void)
{
	ReleaseAllTextures();
}

HRESULT TextureManager::CreateTextureFromFile(ID3D12Device* pD3D, std::wstring labelName, const wchar_t* filename, int mipmapLevel/*MipMap用引数追加*/)
{
	HRESULT hr;

	if (m_textureDB.find(labelName) != m_textureDB.end() && m_textureDB[labelName].get() != nullptr)
		return S_OK;	//既に読み込み済みの物はスキップ

	m_textureDB[labelName].reset(new Texture2DContainer());

	//======MipMap
	hr = CreateTextureFromFile(pD3D, m_textureDB[labelName].get(), filename);

	bool genMipMap = false;
	if (mipmapLevel <= 1)
	{
		mipmapLevel = 1;	//MipMapが無し = Level:1
	}
	else
	{
		genMipMap = true;	//MipMap生成モードをON
	}

	m_textureDB[labelName]->m_miplevels = mipmapLevel;	//miplevelsを構造体に保存
	hr = CreateTextureFromFile(pD3D, m_textureDB[labelName].get(), filename, genMipMap);
	//======MipMap End

	return hr;
}

//======MipMap
void TextureManager::InitMipMapGenerator()
{
	//MipMap用パイプラインステートの作成
	MyGameEngine* engine = MyAccessHub::getMyGameEngine();
	ID3D12Device* pD3D = engine->GetDirect3DDevice();

	CD3DX12_DESCRIPTOR_RANGE srvCbvRanges[2];
	CD3DX12_ROOT_PARAMETER rootParameters[3];

	srvCbvRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
	srvCbvRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);	//UAV

	rootParameters[0].InitAsConstants(2, 0);	//定数バッファ
	rootParameters[1].InitAsDescriptorTable(1, &srvCbvRanges[0]);
	rootParameters[2].InitAsDescriptorTable(1, &srvCbvRanges[1]);

	//サンプラー
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.MaxAnisotropy = 0;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	samplerDesc.ShaderRegister = 0;
	samplerDesc.RegisterSpace = 0;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;		//Memo:コンピュートシェーダで使う時はALL

	//RootSignature
	ID3DBlob* signature;
	ID3DBlob* error;
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;

	rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &samplerDesc,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		&signature, &error);	//RootSignature設定のビルド

	//m_mipMapRootSignatureにRootSignature作成
	pD3D->CreateRootSignature(0, signature->GetBufferPointer(),
		signature->GetBufferSize(), IID_PPV_ARGS(m_mipMapRootSignature.GetAddressOf()));

	//シェーダ読み込み用構造体
	struct
	{
		byte* data;
		uint32_t size;
	} computeShader;	//このまま使う

	//csoの読み込み
	ReadDataFromFile(L"Resources/shaders/CreateMipMapShader.cso",
		&computeShader.data, &computeShader.size);

	//コンピュートシェーダのパイプラインステートを作成
	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = m_mipMapRootSignature.Get();
	psoDesc.CS = { reinterpret_cast<UINT8*>(computeShader.data),computeShader.size };

	//Memo:作成メソッドも少し違う。ただし作成されるオブジェクトの型は同じ
	pD3D->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(m_psoMipMaps.GetAddressOf()));
}
//======MipMap End

//======MipMap
void TextureManager::GenerateMipMap()
{
	//MipMap生成パイプラインの実行
	MyGameEngine* engine = MyAccessHub::getMyGameEngine();
	ID3D12Device* pD3D = engine->GetDirect3DDevice();
	Texture2DContainer* texture2D;

	struct DWParam
	{
		DWParam(FLOAT f) : Float(f) {}
		DWParam(UINT u) : Uint(u) {}

		void operator = (FLOAT f) { Float = f; }
		void operator = (UINT u) { Uint = u; }

		union
		{
			FLOAT Float;
			UINT Uint;
		};
	};

	//事前に作成するMipMapテクスチャの合計を算出して、一気にDescriptorHeapを作成
	UINT32 requiredHeapSize = 0;
	for (auto ite = m_mipmapGenerateList.begin(); ite != m_mipmapGenerateList.end(); ite++)
	{
		texture2D = *ite;
		requiredHeapSize += texture2D->m_miplevels - 1;
	}

	//テクスチャの登録がない or MipMapが必要なテクスチャがない為終了
	if (requiredHeapSize == 0)
	{
		m_mipmapGenerateList.clear();
		return;
	}

	//必要数分DescriptorHeapを作成
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 2 * requiredHeapSize;	//コピー元と生成先で * 2
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	ID3D12DescriptorHeap* descriptorHeap;
	pD3D->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap));

	//Heapのずらし分を事前取得
	UINT descriptorSize =
		pD3D->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//シェーダリソースの準備
	//通常のテクスチャ用
	D3D12_SHADER_RESOURCE_VIEW_DESC srcTextureSRVDesc = {};
	srcTextureSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srcTextureSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	//UAV用
	D3D12_UNORDERED_ACCESS_VIEW_DESC destTextureUAVDesc = {};
	destTextureUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

	//コマンドリスト準備
	//エンジンからSwapChainが使うCommandAllocatorを取得
	ID3D12CommandAllocator* cmdAL = engine->GetCurrentCommandAllocator();

	//コマンドリストを生成
	//一度実行したら二度目は不要で、Allocatorごとに保持し続けるのもメモリがもったいないので、
	//ReleaseAndGetAddressOfで毎回前回のコマンドを破棄して生成し直している
	ThrowIfFailed(pD3D->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAL,
		m_psoMipMaps.Get(), IID_PPV_ARGS(m_MipMapCmdList.ReleaseAndGetAddressOf())));

	//コマンドリストにRootSignatureやPipelineState、DescriptorHeapをセット
	//Memo:コンピュートシェーダ用のRootSignatureのメソッドが違う
	m_MipMapCmdList->SetComputeRootSignature(m_mipMapRootSignature.Get());
	m_MipMapCmdList->SetPipelineState(m_psoMipMaps.Get());
	m_MipMapCmdList->SetDescriptorHeaps(1, &descriptorHeap);

	//DescriptorHeapのCPU先頭アドレスを登録したCPUハンドルを作成
	CD3DX12_CPU_DESCRIPTOR_HANDLE currentCPUHandle(
		descriptorHeap->GetCPUDescriptorHandleForHeapStart(), 0, descriptorSize);

	//DescriptorHeapのGPU先頭アドレスを登録したGPUハンドルを作成
	CD3DX12_GPU_DESCRIPTOR_HANDLE currentGPUHandle(
		descriptorHeap->GetGPUDescriptorHandleForHeapStart(), 0, descriptorSize);

	//ループ処理本番
	for (auto textureIte = m_mipmapGenerateList.begin();
		textureIte != m_mipmapGenerateList.end(); textureIte++)
	{
		texture2D = *textureIte;
		if (texture2D->m_miplevels <= 1)
			continue;

		//生成元テクスチャのSRVを作成。MipMapのレベルが進むごとに必要数が増える
		D3D12_RESOURCE_DESC renderTexDesc{};
		renderTexDesc.Format = texture2D->texFormat;
		renderTexDesc.Width = texture2D->fWidth;
		renderTexDesc.Height = texture2D->fHeight;
		renderTexDesc.MipLevels = texture2D->m_miplevels;
		renderTexDesc.DepthOrArraySize = 1;
		renderTexDesc.SampleDesc.Count = 1;
		renderTexDesc.SampleDesc.Quality = 0;
		renderTexDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		renderTexDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

		const CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
		pD3D->CreateCommittedResource(
			&defaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&renderTexDesc,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			nullptr,
			IID_PPV_ARGS(texture2D->m_pMipmap.ReleaseAndGetAddressOf())
		);

		//本体のテクスチャをコピー元モードに + 今作ったシェーダリソースをコピー先モードに切り替え
		CD3DX12_RESOURCE_BARRIER tra[2];
		tra[0] = CD3DX12_RESOURCE_BARRIER::Transition(texture2D->m_pTexture.Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE);
		tra[1] = CD3DX12_RESOURCE_BARRIER::Transition(texture2D->m_pMipmap.Get(),
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
		m_MipMapCmdList->ResourceBarrier(2, tra);

		//Copy MipMap生成元テクスチャに元テクスチャの内容をコピー
		m_MipMapCmdList->CopyResource(texture2D->m_pMipmap.Get(), texture2D->m_pTexture.Get());

		//MipMapLevelに応じてループを繰り返し、MipMapテクスチャを順次生成していく
		for (uint32_t TopMip = 0; TopMip < texture2D->m_miplevels - 1; TopMip++)
		{
			//コピー状態になっているそれぞれのリソースをコンピュートシェーダ用モードに切り替え
			tra[0] = CD3DX12_RESOURCE_BARRIER::Transition(texture2D->m_pTexture.Get(),
				D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			tra[1] = CD3DX12_RESOURCE_BARRIER::Transition(texture2D->m_pMipmap.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

			m_MipMapCmdList->ResourceBarrier(2, tra);
			//MipMapのテクスチャサイズを計算
			//WIDTH、HEIGHTともにビットシフトで半分のサイズにして、1とどちらが大きいか比較
			uint32_t dstWidth = (std::max)(static_cast<int>(texture2D->fWidth) >> (TopMip + 1), 1);
			uint32_t dstHeight = (std::max)(static_cast<int>(texture2D->fHeight) >> (TopMip + 1), 1);

			//常に新しいMipMapテクスチャが読み込み元になるので新規シェーダリソースが必要
			srcTextureSRVDesc.Format = texture2D->texFormat;
			srcTextureSRVDesc.Texture2D.MipLevels = 1;
			srcTextureSRVDesc.Texture2D.MostDetailedMip = TopMip;
			pD3D->CreateShaderResourceView(texture2D->m_pMipmap.Get(),
				&srcTextureSRVDesc, currentCPUHandle);
			currentCPUHandle.Offset(1, descriptorSize); //新しいSRVが出来たのでアドレスをずらす

			//書き込み先のテクスチャも更新されたのでUAVを再生成
			destTextureUAVDesc.Format = texture2D->texFormat;
			destTextureUAVDesc.Texture2D.MipSlice = TopMip + 1; //ここが変わる
			pD3D->CreateUnorderedAccessView(texture2D->m_pTexture.Get(), nullptr,
				&destTextureUAVDesc, currentCPUHandle);
			currentCPUHandle.Offset(1, descriptorSize); //新しいUAVが出来たのでアドレスをずらす

			//定数バッファにWidthとHeightを登録
			m_MipMapCmdList->SetComputeRoot32BitConstant(0, DWParam(1.0f / dstWidth).Uint, 0);
			m_MipMapCmdList->SetComputeRoot32BitConstant(0, DWParam(1.0f / dstHeight).Uint, 1);

			//生成した新しいSRVとUAVをコンピュートシェーダに登録
			m_MipMapCmdList->SetComputeRootDescriptorTable(1, currentGPUHandle);
			currentGPUHandle.Offset(1, descriptorSize); //シェーダに渡すのはGPUハンドル
			m_MipMapCmdList->SetComputeRootDescriptorTable(2, currentGPUHandle);
			currentGPUHandle.Offset(1, descriptorSize);

			//Dispatch がコンピュートシェーダの実行コマンド
			//Memo:コンピュートシェーダは3次元スレッド
			m_MipMapCmdList->Dispatch((std::max)(dstWidth / 8, 1u),
				(std::max)(dstHeight / 8, 1u), 1);

			//UAVの書き込み終了待ち
			tra[1] = CD3DX12_RESOURCE_BARRIER::UAV(texture2D->m_pTexture.Get());
			m_MipMapCmdList->ResourceBarrier(1, &tra[1]); //バリア変更で待機になる

			//今出来たMipMapを元に新しいMipMapが必要になるので、またコピー
			tra[0] = CD3DX12_RESOURCE_BARRIER::Transition(texture2D->m_pTexture.Get(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
			tra[1] = CD3DX12_RESOURCE_BARRIER::Transition(texture2D->m_pMipmap.Get(),
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
			m_MipMapCmdList->ResourceBarrier(2, tra);

			//Copy
			m_MipMapCmdList->CopyResource(texture2D->m_pMipmap.Get(),
				texture2D->m_pTexture.Get());
		}

		// MipMapループが終わったので、元テクスチャのモードをShaderResourceに戻す
		tra[0] = CD3DX12_RESOURCE_BARRIER::Transition(texture2D->m_pTexture.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		m_MipMapCmdList->ResourceBarrier(1, &tra[0]);

	}

	//コマンドリストのclose
	m_MipMapCmdList->Close();
	//MipMap生成リストをクリア
	m_mipmapGenerateList.clear();
	//コマンドリスト実行
	engine->GetCommandQueue()->ExecuteCommandLists(1, (ID3D12CommandList* const*)m_MipMapCmdList.GetAddressOf());
	//GPU処理待ち
	engine->WaitForGpu();
}
//======MipMap End

//======MultiPath Rendering
HRESULT TextureManager::CreateRenderTargetTexture(ID3D12Device* pD3D, std::wstring labelName, UINT width, UINT height, DXGI_FORMAT dxgiFormat, UINT cpuFlag)
{
	HRESULT hr = E_FAIL;

	m_textureDB[labelName].reset(new Texture2DContainer());

	D3D12_RESOURCE_DESC renderTexDesc{};
	renderTexDesc.Format = dxgiFormat;
	renderTexDesc.Width = width;
	renderTexDesc.Height = height;
	renderTexDesc.MipLevels = 1;
	renderTexDesc.DepthOrArraySize = 1;
	renderTexDesc.SampleDesc.Count = 1;
	renderTexDesc.SampleDesc.Quality = 0;
	renderTexDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	renderTexDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	float clColor[4] = { 0, 0, 0, 0 };
	D3D12_CLEAR_VALUE clVal = CD3DX12_CLEAR_VALUE(dxgiFormat, clColor);

	Texture2DContainer* texbuff = m_textureDB[labelName].get();
	texbuff->m_subresouceData.pData = nullptr;
	texbuff->m_subresouceData.SlicePitch = width * height;
	texbuff->m_subresouceData.RowPitch = height;

	texbuff->texFormat = dxgiFormat;
	texbuff->fWidth = (float)width;
	texbuff->fHeight = (float)height;

	const CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);

	hr = pD3D->CreateCommittedResource(
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&renderTexDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		&clVal,
		IID_PPV_ARGS(texbuff->m_pTexture.GetAddressOf())
	);

	if (FAILED(hr))
	{
		ReleaseTexture(labelName);
	}

	ThrowIfFailed(hr);

	NAME_D3D12_OBJECT(m_textureDB[labelName]->m_pTexture);

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = 1;
	rtvHeapDesc.NodeMask = 0;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(pD3D->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(texbuff->descHeap.GetAddressOf())));

	//RTV作成
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(texbuff->descHeap->GetCPUDescriptorHandleForHeapStart());
	pD3D->CreateRenderTargetView(texbuff->m_pTexture.Get(), nullptr, rtvHandle);

	texbuff->rtvDesc.ptr = rtvHandle.ptr;

	return hr;
}
//======MultiPath Rendering End

HRESULT TextureManager::UploadCreatedTextures(ID3D12Device* pD3D, ID3D12GraphicsCommandList* pCmdList, ID3D12CommandQueue* pCmdQueue)
{
	HRESULT hres = E_FAIL;

	for (auto const &pair : m_textureDB)
	{
		Texture2DContainer* txbuff = pair.second.get();

		if (!txbuff->m_uploaded) continue;

		const UINT subresoucesize = 1;

		UpdateSubresources(pCmdList,
			txbuff->m_pTexture.Get(),
			txbuff->m_pTextureUploadHeap.Get(),
			0,
			0,
			subresoucesize,
			&txbuff->m_subresouceData);

		CD3DX12_RESOURCE_BARRIER tra = CD3DX12_RESOURCE_BARRIER::Transition(txbuff->m_pTexture.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		pCmdList->ResourceBarrier(1, &tra);

		//Uploadが終わったらUploadHeapは不要
		pCmdList->DiscardResource(txbuff->m_pTextureUploadHeap.Get(), nullptr);

		txbuff->m_uploaded = false;

	}

	// Close the command list and execute it to begin the initial GPU setup.
	ThrowIfFailed(hres = pCmdList->Close());
	ID3D12CommandList* ppCommandLists[] = { pCmdList };
	pCmdQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	return hres;
}

void TextureManager::ReleaseTexture(std::wstring labelName, bool removeSlot)
{
	if (m_textureDB[labelName] != nullptr)
	{
		ReleaseTexObj(m_textureDB[labelName].get());

		if (removeSlot)
		{
			m_textureDB.erase(labelName);
		}
		else
		{
			m_textureDB[labelName].reset();
		}
	}
}

void TextureManager::ReleaseAllTextures(void)
{
	for (auto const& pair : m_textureDB)
	{
		ReleaseTexObj(pair.second.get());
	}

	m_textureDB.clear();
}

void TextureManager::CreateTextureSRV(ID3D12Device* pD3D, ID3D12DescriptorHeap* pSrvHeap, UINT slotNo, std::wstring texLabel)
{
	if (m_textureDB[texLabel] != nullptr)
	{
		CreateTextureSRV(pD3D, pSrvHeap, slotNo, m_textureDB[texLabel].get());
	}
}

void TextureManager::CreateTextureSRV(ID3D12Device* pD3D, ID3D12DescriptorHeap* pSrvHeap, UINT slotNo, Texture2DContainer* txbuff)
{
	UINT offset = pD3D->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	size_t startAddr = pSrvHeap->GetCPUDescriptorHandleForHeapStart().ptr;

	D3D12_CPU_DESCRIPTOR_HANDLE heapHandle;

	// Describe and create a SRV for the texture.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = txbuff->texFormat;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	//======MipMap
	srvDesc.Texture2D.MipLevels = txbuff->m_miplevels;

	heapHandle.ptr = startAddr + offset * slotNo;

	pD3D->CreateShaderResourceView(txbuff->m_pTexture.Get(),
		&srvDesc,
		heapHandle);
  
}

Texture2DContainer* TextureManager::GetTexture(std::wstring labelName)
{
	if (m_textureDB[labelName] != nullptr)
	{
		return m_textureDB[labelName].get();
	}

	return nullptr;
}

TextureManager::~TextureManager()
{
	DestructTextureManager();
}

HRESULT TextureManager::CreateTextureFromFile(ID3D12Device* pD3D, Texture2DContainer* txbuff, const wchar_t* filename, bool genMipmap)
{
	HRESULT hres = E_FAIL;

	ReleaseTexObj(txbuff);

	enum TexMode
	{
		PNG,
		TGA,
		NONE,
	};

	TexMode texMode = TexMode::NONE;

	if (wcsstr(filename, L".png") != nullptr)
	{
		hres = DirectX::LoadFromWICFile(filename, DirectX::WIC_FLAGS_NONE, &txbuff->m_metaData, txbuff->m_scImage);

		texMode = TexMode::PNG;
	}
	else if (wcsstr(filename, L".tga") != nullptr)
	{
		//TGA読み込みメソッドの実行
		hres = DirectX::LoadFromTGAFile(filename, &txbuff->m_metaData, txbuff->m_scImage);

		//テクスチャモードをTGAに
		texMode = TexMode::TGA;
	}

	//======MipMap
	if (SUCCEEDED(hres))
	{
		//metadataを修正して、テクスチャにmipmapLevelを埋め込む
		txbuff->m_metaData.mipLevels = txbuff->m_miplevels;

		//テクスチャ作成 MipMapのためにUAV対応をする必要があり、フォーマットを限定する
		if (genMipmap)
		{
			switch (txbuff->m_metaData.format)
			{
			case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			case DXGI_FORMAT_R8G8B8A8_UNORM:
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:	//SRGBだとUNORDERED_ACCESSが出来ない
				txbuff->m_metaData.format = DXGI_FORMAT_R8G8B8A8_UNORM;
				break;

			case DXGI_FORMAT_B8G8R8A8_TYPELESS:
			case DXGI_FORMAT_B8G8R8A8_UNORM:
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:	//SRGBだとUNORDERED_ACCESSが出来ない
				txbuff->m_metaData.format = DXGI_FORMAT_B8G8R8A8_UNORM;
				break;

			default:
				break;
			}

			txbuff->texFormat = txbuff->m_metaData.format;

			//MipMapの為に後からテクスチャを書き換える必要があるため、UNORDERED_ACCESSモードで生成
			hres = DirectX::CreateTextureEx(pD3D, txbuff->m_metaData,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
				CREATETEX_DEFAULT,
				(ID3D12Resource**)txbuff->m_pTexture.GetAddressOf());

			if (SUCCEEDED(hres))
				m_mipmapGenerateList.push_back(txbuff);	//MipMap生成リストに追加
		}
		else //if(genMipMap)
		{
			hres = DirectX::CreateTextureEx(pD3D, txbuff->m_metaData,
				D3D12_RESOURCE_FLAG_NONE,	//通常のテクスチャはFLAG_NONE
				CREATETEX_DEFAULT,
				(ID3D12Resource**)txbuff->m_pTexture.GetAddressOf());
		}
	}
	else //if(SUCCEEDED(hres))
	{
		return E_FAIL;
	}

	//Texture2DContainerにデータを登録
	txbuff->m_subresouceData.pData = txbuff->m_scImage.GetPixels();
	txbuff->m_subresouceData.SlicePitch = txbuff->m_scImage.GetPixelsSize();
	txbuff->m_subresouceData.RowPitch = txbuff->m_subresouceData.SlicePitch / txbuff->m_metaData.width;
	//======MipMap End

	if (SUCCEEDED(hres))
	{

		D3D12_RESOURCE_DESC texDesc;
		texDesc = txbuff->m_pTexture->GetDesc();
		txbuff->fWidth = static_cast<float>(texDesc.Width);
		txbuff->fHeight = static_cast<float>(texDesc.Height);
		txbuff->texFormat = texDesc.Format;

		txbuff->m_uploaded = true;

		UINT64 uploadBufferSize;

		switch (texMode)
		{
		case TexMode::PNG:
			uploadBufferSize = txbuff->m_subresouceData.SlicePitch;
			break;
		case TexMode::TGA:
			uploadBufferSize = txbuff->m_scImage.GetPixelsSize();
			break;
		}

		// Create the GPU upload buffer.
		CD3DX12_HEAP_PROPERTIES upHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
		ThrowIfFailed(pD3D->CreateCommittedResource(
			&upHeap,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(txbuff->m_pTextureUploadHeap.GetAddressOf())));

	}

	return hres; 
}
