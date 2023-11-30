#include <algorithm>

#include <MyAccessHub.h>
#include <MyGameEngine.h>
#include "FBXCharacterData.h"

using namespace fbxsdk;
using namespace DirectX;

void FBXCharacterData::ResetBoneMatrix(int clCount)
{
	for (int i = 0; i < clCount; i++)
	{
		memset(&m_F4X4Matrix[i], 0, sizeof(XMFLOAT4X4));

		m_F4X4Matrix[i]._11 = 1.0f;
		m_F4X4Matrix[i]._22 = 1.0f;
		m_F4X4Matrix[i]._33 = 1.0f;
		m_F4X4Matrix[i]._44 = 1.0f;
	}
}

FBXCharacterData::FBXCharacterData()
{
	AddConstantBuffer(sizeof(XMMATRIX), nullptr);	//For ModelMatrix
}

HRESULT FBXCharacterData::SetMainFBX(const std::wstring fbxId)
{
	FBXDataContainerSystem* fbxContSys = FBXDataContainerSystem::GetInstance();
	m_mainFbx = fbxContSys->GetModelFbx(fbxId);

	if (m_mainFbx == nullptr)
		return E_FAIL;

	int clCount = m_mainFbx->GetClusterCount();

	m_boneConvertIdList.clear();
	m_F4X4Matrix.clear();

	if (clCount > 0)
	{
		int curCbuff = m_cbuffCount;
		AddConstantBuffer(sizeof(XMFLOAT4X4) * clCount, nullptr);
		m_mainFbx->SetCBuffIndex(curCbuff);

		m_boneConvertIdList.resize(clCount);
		m_F4X4Matrix.resize(clCount);

		ResetBoneMatrix(clCount);
	}

	return S_OK;
}

void FBXCharacterData::SetAnime(std::wstring animeLabel)
{
	if (animeLabel != m_currentAnimeLabel)
	{
		FBXDataContainerSystem* fbxContSys = FBXDataContainerSystem::GetInstance();
		FBXDataContainer* animeCont = fbxContSys->GetAnimeFbx(animeLabel);

		if (animeCont != nullptr)
		{
			MeshContainer* meshCont = nullptr;

			m_currentAnimeLabel = animeLabel;
			m_animeTime = 0;

			animeCont->GetFbxScene()->SetCurrentAnimationStack(animeCont->GetAnimeStack());

			m_animeFbx = animeCont;

			//IDリスト更新
			int clCount = m_mainFbx->GetClusterCount();
			for (int i = 0; i < clCount; i++)
			{
				m_boneConvertIdList[i] = m_animeFbx->GetNodeId(m_mainFbx->GetBoneName(i));
			}

			ResetBoneMatrix(clCount);	//ボーン構成が異なる場合に前のアニメのデータが残るためリセット
			UpdateAnimation(0);
		}

	}
}

void FBXCharacterData::UpdateAnimation()
{
	assert(m_animeFbx);
	int frames = m_animeFbx->GetAnimeFrames();
	m_animeTime++;
	if (m_animeTime >= frames)
	{
		m_animeTime -= frames;
	}
	UpdateAnimation(m_animeTime);
}

void FBXCharacterData::UpdateAnimation(int frameCount)
{
	double nowTime = m_animeFbx->GetPeriodTime() * frameCount;

	FbxTime currentTime;
	currentTime.SetSecondDouble(nowTime);

	//ボーンデータ更新
	m_mainFbx->GetAnimatedMatrix(currentTime, m_animeFbx->GetFbxScene(), m_boneConvertIdList, m_F4X4Matrix);

}

//当たり判定関係
bool FBXCharacterData::MakeAABB(UINT index, HitAABB& aabb)
{
	MeshContainer* mesh = m_mainFbx->GetMeshContainer(index);

	if (mesh == nullptr || mesh->m_vertexCount < 2) return false;

	aabb.setAABBMinMax(mesh->m_vtxMin, mesh->m_vtxMax);

	return true;
}

bool FBXCharacterData::MakeSphere(UINT index, HitSphere& sphere)
{
	MeshContainer* mesh = m_mainFbx->GetMeshContainer(index);

	if (mesh == nullptr || mesh->m_vertexCount < 2) return false;

	sphere.setSphereMinMax(mesh->m_vtxMin, mesh->m_vtxMax);

	return true;
}
XMFLOAT4X4* FBXCharacterData::GetAnimatedMatrixData()
{
	return m_F4X4Matrix.data();
}
// ここまで
