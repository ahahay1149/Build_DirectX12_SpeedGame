#pragma once
#include <Windows.h>
#include <MyAccessHub.h>
#include <CharacterData.h>

#include <memory>
#include <vector>
#include <unordered_map>
#include <wrl/client.h>

#include <fbxsdk.h>

#include "HitShapes.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

struct FbxVertex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;

	//======Normal Map
	XMFLOAT3 tangent;
	//======Normal Map End

	XMFLOAT4 color;
	XMFLOAT2 uv;
};

// SkinAnime 01
//スキンアニメ用頂点データ構造体追加
struct FbxSkinAnimeParams
{
	//SkinMesh UnityChanは5点以上の影響ボーンがあるクラスタがあるので2つ必要
	XMUINT4	indices0;
	XMUINT4	indices1;
	XMFLOAT4 weight0;
	XMFLOAT4 weight1;
};

struct FbxSkinAnimeVertex
{
	FbxVertex			vertex;
	FbxSkinAnimeParams	skinvalues;
};
// ここまで

//=======Specular
struct FbxMaterialInfo
{
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;

	UINT TextureFlag;
};
//=======Specular End

enum class FBX_TEXTURE_TYPE
{
	//=======Specular
	FBX_DIFFUSE = 0x01,
	FBX_NORMAL = 0x02,
	FBX_SPECUAR = 0x04,
	FBX_FALLOFF = 0x08,
	FBX_REFLECTIONMAP = 0x10,

	FBX_UNKNOWN = 0x80
	//=======Specular End
};

class FBXDataContainer;

class MeshContainer
{
protected:

	FbxMesh* m_mesh;

	const char* m_meshNodeName;

	// SkinAnime 02
	//スキンアニメ用メンバ変数追加
	FbxAMatrix m_IBaseMatrix;

	UINT	m_skinCount = 0;
	// ここまで

public:
	std::wstring m_MaterialId = L"";
	std::wstring m_MeshId = L"";

	UINT	m_vertexCount = 0;

	std::vector<FbxVertex>	m_vertexData;
	std::vector<ULONG>		m_indexData;

	//頂点データの最大値と最小値
	XMFLOAT3				m_vtxMin;
	XMFLOAT3				m_vtxMax;

	~MeshContainer();

	void setFbxMesh(FbxMesh* mesh);

	const char* getMeshNodeName()
	{
		return m_meshNodeName;
	}

	FbxMesh* getFbxMesh()
	{
		return m_mesh;
	}

	// SkinAnime 03
	//スキンアニメ用メソッド追加
	UINT GetSkinCount()
	{
		return m_skinCount;
	}

	void InitSkinList(int skinCount);
	// ここまで

};

class MaterialContainer
{
private:
	bool	m_uniqueTextures = true;
	//=======Specular
	ComPtr<ID3D12Resource> m_d3dresource;
	//=======Specular End

public:
	//=======Specular
	FbxMaterialInfo materialInfo;
	//=======Specular End

	std::vector<std::wstring>	m_diffuseTextures;
	std::vector<std::wstring>	m_normalTextures;
	std::vector<std::wstring>	m_specularTextures;
	std::vector<std::wstring>	m_falloffTextures;
	std::vector<std::wstring>	m_reflectionMapTextures;

	MaterialContainer()
	{
		//=======Specular
		materialInfo.ambient.x = 1.0f;
		materialInfo.diffuse.x = 1.0f;
		materialInfo.specular.x = 1.0f;
		materialInfo.ambient.y = 1.0f;
		materialInfo.diffuse.y = 1.0f;
		materialInfo.specular.y = 1.0f;
		materialInfo.ambient.z = 1.0f;
		materialInfo.diffuse.z = 1.0f;
		materialInfo.specular.z = 1.0f;
		materialInfo.ambient.w = 1.0f;
		materialInfo.diffuse.w = 1.0f;
		materialInfo.specular.w = 1.0f;

		materialInfo.TextureFlag = 0;

		MyGameEngine* engine = MyAccessHub::getMyGameEngine();
		engine->CreateConstantBuffer(m_d3dresource.GetAddressOf(), &materialInfo, sizeof(FbxMaterialInfo));
		//=======Specular End

		m_diffuseTextures.clear();
		m_normalTextures.clear();
		m_specularTextures.clear();
		m_falloffTextures.clear();
		m_reflectionMapTextures.clear();
	}

	void setUniqueTextureFlag(bool flg)
	{
		m_uniqueTextures = flg;
	}

	~MaterialContainer();

	void setAmbient(float r, float g, float b, float factor)
	{
		//=======Specular
		materialInfo.ambient.x = r;
		materialInfo.ambient.y = g;
		materialInfo.ambient.z = b;
		materialInfo.ambient.w = factor;
		//=======Specular End
	}

	void setDiffuse(float r, float g, float b, float factor)
	{
		//=======Specular
		materialInfo.diffuse.x = r;
		materialInfo.diffuse.y = g;
		materialInfo.diffuse.z = b;
		materialInfo.diffuse.w = factor;
		//=======Specular End
	}

	void setSpecular(float r, float g, float b, float factor)
	{
		//=======Specular
		materialInfo.specular.x = r;
		materialInfo.specular.y = g;
		materialInfo.specular.z = b;
		materialInfo.specular.w = factor;
		//=======Specular End
	}

	//=======Specular
	void UpdateD3DResource();

	ID3D12Resource* GetMaterialInfoResource()
	{
		return m_d3dresource.Get();
	}
	//=======Specular End
};

class FBXDataContainer
{
private:

	// SkinAnime04
	//スキンアニメ用メンバ変数追加
	FbxScene* m_pFbxScene;	//FBXファイルデータ本体

	LONG	m_animeFrames;	//アニメフレーム数
	double m_startTime;		//アニメ開始時間
	double m_endTime;		//アニメ終了時間

	double m_timePeriod;	//1フレームの時間 / 60fpsで固定

	FbxAnimStack* m_animeStack;	//FbxSdkの構造体。アニメーションレイヤーデータ

	int		m_clusterCount;	//ボーンノード（関節点）の数
	int		m_cbuffIndex;	//アニメデータ用定数バッファインデックス

	std::vector<const char*>		m_boneNameList;	//ボーン名の配列
	std::vector<FbxAMatrix>			m_IboneMatrix;	//計算用元初期ボーンの逆行列
	// ここまで

	std::vector<const char*> m_nodeNameList;
	std::vector<unique_ptr<MeshContainer>> m_pMeshContainer;
	std::unordered_map<std::wstring, unique_ptr<MaterialContainer>> m_pMaterialContainer;

	//頂点データの最大値と最小値
	XMFLOAT3	m_vtxTotalMin;
	XMFLOAT3	m_vtxTotalMax;

	FBX_TEXTURE_TYPE GetTextureType(const fbxsdk::FbxBindingTableEntry& entryTable);

	HRESULT ReadFbxToMeshContainer(const std::wstring id, FbxMesh* pMesh);
	HRESULT LoadMaterial(const std::wstring id, FbxSurfaceMaterial* material);

	HRESULT LoadTextureFromMaterial(const std::wstring matName, const std::wstring id, FBX_TEXTURE_TYPE texType, const FbxProperty* fbxProp);

	// SkinAnime05
	//スキンアニメ用クラスター数アクセサ追記
	int GetClusterId(FbxCluster* pCluster);
	int GetClusterId(FbxNode* pNode);
	// ここまで

public:
	~FBXDataContainer()
	{
		// SkinAnime06
		//スキンアニメ用メンバ削除処理
		if (m_pFbxScene != nullptr)
		{
			m_pFbxScene->Destroy();
			m_pFbxScene = nullptr;
		}

		m_boneNameList.clear();
		m_IboneMatrix.clear();
		// ここまで

		m_pMeshContainer.clear();
		m_pMaterialContainer.clear();
	}

	HRESULT LoadFBX(const std::wstring fileName, const std::wstring id, bool noMaterial);

	XMFLOAT3 GetFbxMin()
	{
		return m_vtxTotalMin;
	}

	XMFLOAT3 GetFbxMax()
	{
		return m_vtxTotalMax;
	}

	MeshContainer* GetMeshContainer(int index)
	{
		if (m_pMeshContainer.size() <= index)
		{
			return nullptr;
		}

		return m_pMeshContainer[index].get();
	}

	int GetMeshCount()
	{
		return m_pMeshContainer.size();
	}

	MaterialContainer* GetMaterialContainer(const std::wstring& matName)
	{
		if (m_pMaterialContainer[matName] != nullptr)
			return m_pMaterialContainer[matName].get();

		return nullptr;
	}

	int GetNodeId(const char* nodeName);
	int GetMeshId(const char* meshName);

	//Phase 3
	const char* GetBoneName(int id);
	int GetBoneId(const char* boneName);

	FbxNode* GetMeshNode(int id);

	fbxsdk::FbxManager* GetFbxManager()
	{
		static fbxsdk::FbxManager* m_spFbxManager = nullptr;

		if (m_spFbxManager == nullptr)
		{
			m_spFbxManager = fbxsdk::FbxManager::Create();
		}

		return m_spFbxManager;
	}

	// SkinAnime07
	//スキンアニメ用メソッド追加
	void GetAnimatedMatrix(const FbxTime& animeTime, FbxScene* animeScene, std::vector<int>& idList, std::vector<XMFLOAT4X4>& resMtxArray);	//アニメFBXボーンマトリクス取得

	FbxScene* GetFbxScene() { return m_pFbxScene; }

	double GetStartTime() { return m_startTime; }
	double GetEndTime() { return m_endTime; }
	double GetPeriodTime() { return m_timePeriod; }

	LONG GetAnimeFrames() { return m_animeFrames; }
	FbxAnimStack* GetAnimeStack() { return m_animeStack; }

	int GetClusterCount()
	{
		return m_clusterCount;
	}

	void SetCBuffIndex(int index)
	{
		m_cbuffIndex = index;
	}

	int GetCBuffIndex()
	{
		return m_cbuffIndex;
	}
	// ここまで
};


class FBXDataContainerSystem
{
private:
	std::unordered_map<std::wstring, std::unique_ptr<FBXDataContainer>> m_modelFbxMap;
	std::unordered_map<std::wstring, std::unique_ptr<FBXDataContainer>> m_animeFbxMap;

	FBXDataContainerSystem()
	{
		m_modelFbxMap.clear();
		m_animeFbxMap.clear();
	}

	~FBXDataContainerSystem()
	{
		m_modelFbxMap.clear();
		m_animeFbxMap.clear();
	}

public:

	static FBXDataContainerSystem* GetInstance()
	{
		static FBXDataContainerSystem* fbxContSys = new FBXDataContainerSystem();

		return fbxContSys;
	}

	HRESULT LoadModelFBX(const std::wstring fileName, const std::wstring id);
	FBXDataContainer* GetModelFbx(const std::wstring id);
	HRESULT LoadAnimationFBX(const std::wstring fileName, const std::wstring id);	//アニメ用FBX読み込み
	FBXDataContainer* GetAnimeFbx(const std::wstring id);							//アニメ用FBXをラベル名で取得

	void ClearModelFBX();
	void ClearAnimeFBX();

	void DeleteModelFBX(std::wstring id);
	void DeleteAnimeFBX(std::wstring id);
};

