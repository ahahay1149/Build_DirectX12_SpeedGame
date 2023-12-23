#pragma once
#include <Windows.h>
#include <d3d12.h>

#include <vector>
#include <unordered_map>

#include <memory>
#include <wrl/client.h>

#include <string>

//DirectXTex追加
#include <DirectXTex.h>


using Microsoft::WRL::ComPtr;

struct Texture2DContainer
{
	ComPtr<ID3D12Resource> m_pTexture;
	ComPtr<ID3D12Resource> m_pTextureUploadHeap;

	bool m_uploaded = false;

	std::unique_ptr<uint8_t[]> m_wicData;
	D3D12_SUBRESOURCE_DATA m_subresouceData;

	//DirectXTex
	DirectX::TexMetadata m_metaData;	//画像フォーマットデータ
	DirectX::ScratchImage m_scImage;	//画像データ本体

	float fWidth;
	float fHeight;
	DXGI_FORMAT texFormat;

	ComPtr<ID3D12DescriptorHeap> descHeap;

	//RTV
	D3D12_CPU_DESCRIPTOR_HANDLE rtvDesc;

	Texture2DContainer()
	{
		rtvDesc.ptr = 0;

		fWidth = 0.0f;
		fHeight = 0.0f;
		texFormat = DXGI_FORMAT_B8G8R8A8_UNORM;

		m_metaData = {};
		m_scImage = {};
		m_subresouceData = {};
	}
};

class TextureManager
{
private:
	std::unordered_map<std::wstring, std::unique_ptr<Texture2DContainer>> m_textureDB;

	HRESULT CreateTextureFromFile(ID3D12Device* g_pD3D, Texture2DContainer* txbuff, const wchar_t* filename, bool genMipmap = false);
	void	ReleaseTexObj(Texture2DContainer* txbuff);
	void	DestructTextureManager(void);

public:

	HRESULT InitTextureManager(void);

	HRESULT CreateTextureFromFile(ID3D12Device* pD3D, std::wstring labelName, const wchar_t* filename);

	//=====MultiPath Rendering
	HRESULT CreateRenderTargetTexture(ID3D12Device* pD3D, std::wstring labelName, UINT width, UINT height,
		DXGI_FORMAT dxgiFormat = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, UINT cpuFlag = 0);
	//=====MultiPath Rendering End

	HRESULT UploadCreatedTextures(ID3D12Device* pD3D, ID3D12GraphicsCommandList* pCmdList, ID3D12CommandQueue* pCmdQueue);

	void ReleaseTexture(std::wstring labelName, bool removeSlot = false);
	void ReleaseAllTextures(void);

	void CreateTextureSRV(ID3D12Device* pD3D, ID3D12DescriptorHeap* pSrvHeap, UINT slotNo, std::wstring texLabel);
	void CreateTextureSRV(ID3D12Device* pD3D, ID3D12DescriptorHeap* pSrvHeap, UINT slotNo, Texture2DContainer* txbuff);

	Texture2DContainer* GetTexture(std::wstring labelName);

	~TextureManager();
};
