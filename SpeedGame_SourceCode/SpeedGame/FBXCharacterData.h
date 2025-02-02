﻿#pragma once
#include <Windows.h>
#include <MyAccessHub.h>
#include <CharacterData.h>

#include <memory>
#include <vector>
#include <unordered_map>
#include <wrl/client.h>

#include <fbxsdk.h>

#include "HitShapes.h"
#include "FBXDataContainerSystem.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

class FBXDataContainer;


class FBXCharacterData : public CharacterData
{
private:

	FBXDataContainer* m_mainFbx;

	// SkinAnime08
	//スキンアニメ用メンバ追加
	FBXDataContainer* m_animeFbx;

	std::wstring m_currentAnimeLabel;	//後から
	LONG	m_animeTime;				//アニメのコマ時間
	// ここまで

	//Phase 3
	std::vector<XMFLOAT4X4>			m_F4X4Matrix;			//アニメーションのupdateで更新されるDirect3D用クラスタマトリクス
	std::vector<int>				m_boneConvertIdList;	//アニメ側のボーン名から見た本体側のボーンID値の配列

	void ResetBoneMatrix(int clCount);

	//======Lighting
	std::wstring m_ambientLight;		//LightSettingManagerから受け取る環境光データのラベル
	std::wstring m_directionalLight;	//LightSettingManagerから受け取る平行光源データのラベル
	//======Lighting End

	//影の出力を制御するフラグ
	bool m_shadowFlg = true;

public:

	//Phase 3
	FBXCharacterData();

	HRESULT SetMainFBX(const std::wstring fbxId);

	FBXDataContainer* GetMainFbx()
	{
		//return m_mainFbx.get();
		return m_mainFbx;
	}

	// SkinAnime09
	//スキンアニメ用FBX読み込みとアニメ実行メソッド関係
	void SetAnime(std::wstring animeLabel);		//再生アニメ指定
	void UpdateAnimation();						//アニメ1フレーム更新
	void UpdateAnimation(int frameCount);		//アニメフレーム指定更新
	// ここまで


	// Hit
	//当たり判定関係
	bool MakeAABB(UINT index, HitAABB& aabb);
	bool MakeSphere(UINT index, HitSphere& sphere);
	// ここまで
	
	//Phase3
	XMFLOAT4X4* GetAnimatedMatrixData();

	//======Lighting
	void SetAmbientLight(std::wstring amb)
	{
		m_ambientLight = amb;
	}

	std::wstring& GetAmbientLight()
	{
		return m_ambientLight;
	}

	void SetDirectionalLight(std::wstring dir)
	{
		m_directionalLight = dir;
	}

	std::wstring& GetDirectionalLight()
	{
		return m_directionalLight;
	}
	//======Lighting End

	//======Shadow Flag
	void SetShadowFlag(bool flg)
	{
		m_shadowFlg = flg;
	}

	bool GetShadowFlag()
	{
		return m_shadowFlg;
	}
	//======Shadow Flag End
};