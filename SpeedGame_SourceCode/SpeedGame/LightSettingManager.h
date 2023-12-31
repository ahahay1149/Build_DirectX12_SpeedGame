#pragma once

#include <string>
#include <unordered_map>

#include <d3d12.h>
#include <DirectXMath.h>

#include <wrl/client.h>
#include <memory>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class DirectionalLightContainer	//平行光源コンテナ
{
public:
	struct DirectionalLightData	//平行光源データの構造体
	{
		XMVECTOR Color;			//光の色
		XMVECTOR Direction;		//光の角度
	};

	DirectionalLightContainer();	//コンストラクタ
	void SetDirectionalLight(XMFLOAT3 color, XMFLOAT3 direction);	//光設定

	DirectionalLightData& GetLightData();	//光データ構造体取得
	ID3D12Resource* GetConstantBuffer();	//シェーダ用Resource取得

	//======Depth Shadow
	// 光源のView行列更新
	void UpdateLightBaseMatrix(XMFLOAT3& eyePos, XMFLOAT3& focusPos);

	XMMATRIX GetLightViewMtx();				//View行列データの取得
	ID3D12Resource* GetLightBaseMtxBuffer();//View行列のシェーダリソースを取得
	//======Depth Shadow End

private:

	//======Depth Shadow
	XMMATRIX m_lightViewMtx;				//View行列
	ComPtr<ID3D12Resource> m_lightMtxBuff;	//View行列のシェーダリソース
	//======Depth Shadow End

	DirectionalLightData m_lightData;	//光データ本体
	ComPtr<ID3D12Resource> m_cBuff;		//シェーダ用リソース

	void UpdateCBuffer();				//光設定時のリソース更新メソッド

};

class AmbientLightContainer		//環境光コンテナ
{
private:
	XMVECTOR m_color;				//RGBカラー
	ComPtr<ID3D12Resource> m_cBuff;	//シェーダリソース

	void UpdateCBuffer();			//設定変更時のシェーダリソース更新

public:
	AmbientLightContainer();		//コンストラクタ
	void SetLight(float r, float g, float b);	//ライト設定float型
	void SetLight(XMFLOAT3 color);				//ライト設定XMVECTOR型

	XMFLOAT3 GetLight()
	{
		XMFLOAT3 col = {XMVectorGetX(m_color),XMVectorGetY(m_color),XMVectorGetZ(m_color)};
		return col;	//XMVECTORなのでXMFLOATに変換して返す
	}

	ID3D12Resource* GetConstantBuffer()
	{
		return m_cBuff.Get();	//シェーダリソースの取得
	}
};

class LightSettingManager
{
private:
	std::unordered_map<std::wstring, std::unique_ptr<AmbientLightContainer> > m_AmbientLights;
	std::unordered_map<std::wstring, std::unique_ptr<DirectionalLightContainer> > m_DirectionalLights;

	//privateで宣言
	LightSettingManager();	//コンストラクタ
	~LightSettingManager();	//デストラクタ

public:
	static LightSettingManager* GetInstance();	//インスタンスはstaticメソッドで取得

	void CreateAmbientLight(std::wstring label, XMFLOAT3 light);							//名前をつけて環境光作成
	void CreateDirectionalLight(std::wstring label, XMFLOAT3 light, XMFLOAT3 direction);	//平行光源版

	AmbientLightContainer* GetAmbientLight(std::wstring label);			//作成時の名前で環境光取得
	DirectionalLightContainer* GetDirectionalLight(std::wstring label);	//平行光源版

	void DeleteAmbientLight(std::wstring label);		//環境光データ削除
	void DeleteDirectionalLight(std::wstring label);	//平行光源データ削除
};

