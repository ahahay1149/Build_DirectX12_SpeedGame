﻿#include <Windows.h>
#include <vector>
#include "TextureManager.h"
#include <WICTextureLoader.h>	//DirectXToolKit 12から


#include "DXSampleHelper.h" //ThrowIfFailed等
#include "d3dx12.h"			//inline色々


#pragma comment(lib, "DirectXTK12.lib")
#pragma comment(lib, "DirectXTex.lib")

using namespace std;
using namespace DirectX;


HRESULT TextureManager::InitTextureManager(void)
{
	m_textureDB.clear();

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

HRESULT TextureManager::CreateTextureFromFile(ID3D12Device* pD3D, std::wstring labelName, const wchar_t* filename)
{
	HRESULT hr;

	if (m_textureDB.find(labelName) != m_textureDB.end() && m_textureDB[labelName].get() != nullptr)
		return S_OK;	//既に読み込み済みの物はスキップ

	m_textureDB[labelName].reset(new Texture2DContainer());

	hr = CreateTextureFromFile(pD3D, m_textureDB[labelName].get(), filename);

	return hr;
}

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
	srvDesc.Texture2D.MipLevels = 1;

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
		hres = LoadWICTextureFromFile(pD3D, filename, (ID3D12Resource**)txbuff->m_pTexture.GetAddressOf(), txbuff->m_wicData, txbuff->m_subresouceData);

		texMode = TexMode::PNG;
	}
	else if (wcsstr(filename, L".tga") != nullptr)
	{
		//TGA読み込みメソッドの実行
		hres = DirectX::LoadFromTGAFile(filename, &txbuff->m_metaData, txbuff->m_scImage);

		if (SUCCEEDED(hres))
		{
			//テクスチャ作成
			hres = DirectX::CreateTexture(pD3D, txbuff->m_metaData, (ID3D12Resource**)txbuff->m_pTexture.GetAddressOf());
		}
		else
		{
			return E_FAIL;
		}

		//Texture2DContainerにデータを登録
		txbuff->m_subresouceData.pData = txbuff->m_scImage.GetPixels();
		txbuff->m_subresouceData.SlicePitch = txbuff->m_scImage.GetPixelsSize();
		txbuff->m_subresouceData.RowPitch = txbuff->m_subresouceData.SlicePitch / txbuff->m_metaData.width;

		//テクスチャモードをTGAに
		texMode = TexMode::TGA;
	}

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
