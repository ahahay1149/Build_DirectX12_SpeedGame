#include <MyAccessHub.h>
#include <HitManager.h>
#include "TerrainComponent.h"

#include "FBXCharacterData.h"

void TerrainComponent::initAction()
{
	FBXCharacterData* fbxChara = static_cast<FBXCharacterData*>(getGameObject()->getCharacterData());
	//fbxChara->SetGraphicsPipeLine(L"StaticFBX");		アニメなしFBXモード
	fbxChara->SetGraphicsPipeLine(L"StaticLambert");	//スキンアニメありLambert

	//TerrainHit01
	m_bloadHit.setAABBMinMax(fbxChara->GetMainFbx()->GetFbxMin(), fbxChara->GetMainFbx()->GetFbxMax());

	MeshContainer* meshCon;
	FbxVertex *vA, *vB, *vC;
	unique_ptr<TerrainPlate> pTerrainPlate;

	int index = 0;
	UINT vcount = 0;

	while ((meshCon = fbxChara->GetMainFbx()->GetMeshContainer(index)) != nullptr)
	{
		//本当はもっとしっかりカプセル化すべき
		vcount = meshCon->m_vertexCount;
		
		for (UINT i = 0; i < vcount; i+=3)
		{
			vA = &meshCon->m_vertexData[meshCon->m_indexData[i]];
			vB = &meshCon->m_vertexData[meshCon->m_indexData[i + 1]];
			vC = &meshCon->m_vertexData[meshCon->m_indexData[i + 2]];

			pTerrainPlate = make_unique<TerrainPlate>();
			pTerrainPlate.get()->setTerrainPlate(vA->position, vB->position, vC->position);

			m_terrainHit.push_back(move(pTerrainPlate));
		}

		index++;
	}
	//TerrainHit01 ここまで
}

bool TerrainComponent::frameAction()
{
	//PipeLineに登録
	getGameObject()->getCharacterData()->GetPipeline()->AddRenerObject(getGameObject()->getCharacterData());
	return true;
}

void TerrainComponent::finishAction()
{
}

void TerrainComponent::hitReaction(GameObject* obj, HitAreaBase* hit)
{
}

//TerrainHit02
bool TerrainComponent::RayCastHit(const XMFLOAT3& rayStart, const XMFLOAT3& rayEnd, XMFLOAT3& hitPosition, XMFLOAT3& hitNormal)
{
	HitRayLine rayLine;

	rayLine.setLine(rayStart, rayEnd, 0);

	return RayCastHit(rayLine, hitPosition, hitNormal);
}

bool TerrainComponent::RayCastHit(HitRayLine& ray, XMFLOAT3& hitPosition, XMFLOAT3& hitNormal)
{
	HitManager* hitMng = MyAccessHub::getMyGameEngine()->GetHitManager();

	//=====動く地形対応
	//地形のアフィン変換をレイに適応する（逆行列をかける）
	HitRayLine trRay = ray;
	CharacterData* chData = getGameObject()->getCharacterData();

	HitRayLine::GetTransformedLine(trRay, chData->GetInverseWorldMatrix());

	XMVECTOR nmlVect = trRay.getNormal();
	//=====動く地形対応 END
	XMFLOAT3 rayNormal = {XMVectorGetX(nmlVect), XMVectorGetY(nmlVect), XMVectorGetZ(nmlVect)};
	XMFLOAT3 tempHit = {};
	XMFLOAT3 tempNml = {};

	bool res = false;

	//=====動く地形対応
	if (hitMng->isHit(&trRay, &m_bloadHit))
	{
		//概形にヒットしたので詳細部分チェック
		for (int i = 0; i < m_terrainHit.size(); i++)
		{
			TerrainPlate* terHit = m_terrainHit[i].get();
			
			//=====動く地形対応
			if (terHit->RayCastHit(trRay, tempHit, tempNml))
			{
				if (!res)
				{
					//初ヒット
					res = true;
					hitPosition = tempHit;
					hitNormal = tempNml;
		 		}
				else
				{
					//より始点に近い座標をヒットとする
					bool ow = false;
					if (rayNormal.x > 0.0f)
					{
						if (hitPosition.x > tempHit.x)
						{
							hitPosition.x = tempHit.x;
							ow = true;
						}
					}
					else
					{
						if (hitPosition.x < tempHit.x)
						{
							hitPosition.x = tempHit.x;
							ow = true;
						}
					}

					if (rayNormal.y > 0.0f)
					{
						if (hitPosition.y > tempHit.y)
						{
							hitPosition.y = tempHit.y;
							ow = true;
						}
					}
					else
					{
						if (hitPosition.y < tempHit.y)
						{
							hitPosition.y = tempHit.y;
							ow = true;
						}
					}

					if (rayNormal.z > 0.0f)
					{
						if (hitPosition.z > tempHit.z)
						{
							hitPosition.z = tempHit.z;
							ow = true;
						}
					}
					else
					{
						if (hitPosition.z < tempHit.z)
						{
							hitPosition.z = tempHit.z;
							ow = true;
						}
					}

					//座標の書き換えがあった
					if (ow)
					{
						//ノーマルを合成
						hitNormal.x = (hitNormal.x + tempNml.x) * 0.5f;
						hitNormal.y = (hitNormal.y + tempNml.y) * 0.5f;
						hitNormal.z = (hitNormal.z + tempNml.z) * 0.5f;
					}
				}
			}
		}
	}

	//=====動く地形対応
	//結果値に現在のアフィン変換を掛ける
	if (res)
	{
		XMVECTOR posV = XMLoadFloat3(&hitPosition);
		XMVECTOR nmlV = XMLoadFloat3(&hitNormal);
		XMMATRIX& mtx = chData->GetWorldMatrix();

		posV = XMVector3Transform(posV, mtx);
		nmlV = XMVector3Transform(nmlV, mtx);
		nmlV = XMVector3Normalize(nmlV);

		hitPosition.x = XMVectorGetX(posV);
		hitPosition.y = XMVectorGetY(posV);
		hitPosition.z = XMVectorGetZ(posV);

		hitNormal.x = XMVectorGetX(nmlV);
		hitNormal.y = XMVectorGetY(nmlV);
		hitNormal.z = XMVectorGetZ(nmlV);
	}
	//=====動く地形対応 END


	return res;
}

void TerrainComponent::TerrainPlate::setTerrainPlate(const XMFLOAT3& pA, const XMFLOAT3& pB, const XMFLOAT3& pC)
{
	mainHit.setTriangle(pA, pB, pC);

	//Bload Hit
	XMFLOAT3 max = pA;
	XMFLOAT3 min = pA;

	//max min check
	for (int i = 0; i < 2; i++)
	{
		XMFLOAT3 target;
		switch (i)
		{
		case 0:
			target = pB;
			break;

		default:
			target = pC;
			break;
		}

		//max check
		if (max.x < target.x)
			max.x = target.x;
		if (max.y < target.y)
			max.y = target.y;
		if (max.z < target.z)
			max.z = target.z;

		//min check
		if (min.x > target.x)
			min.x = target.x;
		if (min.y > target.y)
			min.y = target.y;
		if (min.z > target.z)
			min.z = target.z;

	}

	subHit.setAABBMinMax(min, max);
}

bool TerrainComponent::TerrainPlate::RayCastHit(HitRayLine& ray, XMFLOAT3& hitPosition, XMFLOAT3& hitNormal)
{
	HitManager* hitMng = MyAccessHub::getMyGameEngine()->GetHitManager();

	//bload hit
	if (!hitMng->isHit(&ray, &subHit))
	{
		return false;
	}

	//local hit
	if (hitMng->isHit(&ray, &mainHit))
	{
		XMFLOAT3 pos = hitMng->getLastHitPoint();
		hitPosition.x = pos.x;
		hitPosition.y = pos.y;
		hitPosition.z = pos.z;

		XMVECTOR nml = mainHit.getNormal();
		hitNormal.x = XMVectorGetX(nml);
		hitNormal.y = XMVectorGetY(nml);
		hitNormal.z = XMVectorGetZ(nml);

		return true;
	}

	return false;
}
//TerrainHit02 ここまで
