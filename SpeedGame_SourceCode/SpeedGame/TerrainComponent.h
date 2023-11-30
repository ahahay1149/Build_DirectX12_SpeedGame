#pragma once
#include <HitShapes.h>

#include "GameObject.h"

#include <DirectXMath.h>
#include <vector>

using namespace DirectX;

class TerrainComponent : public GameComponent
{
	//TerrainHit01
	//==========HitSystem========
private:
	//１ポリゴンごとの当たり判定データクラス
	class TerrainPlate
	{
	private:
		HitAABB subHit;			//ブロードヒット
		HitTriangle mainHit;	//ローカルヒット

	public:
		//データ設定用メソッド
		//ポリゴン頂点３点でTerrainPlateデータを作成
		void setTerrainPlate(const XMFLOAT3& pA, const XMFLOAT3& pB, const XMFLOAT3& pC);
		//当たり判定データへのレイキャスト（線分VS三角形）
		bool RayCastHit(HitRayLine& ray, XMFLOAT3& hitPosition, XMFLOAT3& hitNormal);
	};

	HitAABB m_bloadHit;	//地形データ全体のブロードヒット（オープンワールド用）
	std::vector<unique_ptr<TerrainPlate>> m_terrainHit;	//ヒット判定データ配列
	//==========HitSystem========
	//TerrainHit01 ここまで

public:
	void initAction() override;
	bool frameAction() override;
	void finishAction() override;

	void hitReaction(GameObject* obj, HitAreaBase* hit) override;

	//TerrainHit02
	bool RayCastHit(const XMFLOAT3& rayStart, const XMFLOAT3& rayEnd, XMFLOAT3& hitPosition, XMFLOAT3& hitNormal);
	bool RayCastHit(HitRayLine& ray, XMFLOAT3& hitPosition, XMFLOAT3& hitNormal);
	//TerrainHit02 ここまで
};
