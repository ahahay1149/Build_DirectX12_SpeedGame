#include <list>
#include <algorithm>		//for_eachのため

#include "HitManager.h"

//=====================ProtoTypes=================
//一応全ての組み合わせ（８の等比数列の和）分のヒット判定関数を宣言
//使うやつだけ実装予定、ただ宣言は予め行っておく
bool isHitVS(HitQuad* atk, HitQuad* def, XMFLOAT3& resHitPos);			//矩形 VS 矩形
bool isHitVS(HitQuad* atk, HitCircle* def, XMFLOAT3& resHitPos, bool revCheck);		//矩形 VS 円
bool isHitVS(HitQuad* atk, HitTriangle* def, XMFLOAT3& resHitPos, bool revCheck);		//矩形 VS 三角
bool isHitVS(HitQuad* atk, HitAABB* def, XMFLOAT3& resHitPos, bool revCheck);			//矩形 VS AABB
bool isHitVS(HitQuad* atk, HitOBB* def, XMFLOAT3& resHitPos, bool revCheck);			//矩形 VS OBB
bool isHitVS(HitQuad* atk, HitSphere* def, XMFLOAT3& resHitPos, bool revCheck);		//矩形 VS 球
bool isHitVS(HitQuad* atk, HitPillar* def, XMFLOAT3& resHitPos, bool revCheck);		//矩形 VS 円柱
bool isHitVS(HitQuad* atk, HitRayLine* def, XMFLOAT3& resHitPos, bool revCheck);		//矩形 VS 線分

bool isHitVS(HitCircle* atk, HitCircle* def, XMFLOAT3& resHitPos);		//円 VS 円
bool isHitVS(HitCircle* atk, HitTriangle* def, XMFLOAT3& resHitPos, bool revCheck);	//円 VS 三角
bool isHitVS(HitCircle* atk, HitAABB* def, XMFLOAT3& resHitPos, bool revCheck);		//円 VS AABB
bool isHitVS(HitCircle* atk, HitOBB* def, XMFLOAT3& resHitPos, bool revCheck);		//円 VS OBB
bool isHitVS(HitCircle* atk, HitSphere* def, XMFLOAT3& resHitPos, bool revCheck);		//円 VS 球
bool isHitVS(HitCircle* atk, HitPillar* def, XMFLOAT3& resHitPos, bool revCheck);	//円 VS 円柱
bool isHitVS(HitCircle* atk, HitRayLine* def, XMFLOAT3& resHitPos, bool revCheck);	//円 VS 線分

bool isHitVS(HitTriangle* atk, HitTriangle* def, XMFLOAT3& resHitPos);	//三角 VS 三角	*必須
bool isHitVS(HitTriangle* atk, HitAABB* def, XMFLOAT3& resHitPos, bool revCheck);		//三角 VS AABB	*必須
bool isHitVS(HitTriangle* atk, HitOBB* def, XMFLOAT3& resHitPos, bool revCheck);		//三角 VS OBB
bool isHitVS(HitTriangle* atk, HitSphere* def, XMFLOAT3& resHitPos, bool revCheck);	//三角 VS 球	*必須
bool isHitVS(HitTriangle* atk, HitPillar* def, XMFLOAT3& resHitPos, bool revCheck);	//三角 VS 円柱
bool isHitVS(HitTriangle* atk, HitRayLine* def, XMFLOAT3& resHitPos);	//三角 VS 線分	*必須

bool isHitVS(HitAABB* atk, HitAABB* def, XMFLOAT3& resHitPos);			//AABB VS AABB	*必須
bool isHitVS(HitAABB* atk, HitOBB* def, XMFLOAT3& resHitPos, bool revCheck);			//AABB VS OBB
bool isHitVS(HitAABB* atk, HitSphere* def, XMFLOAT3& resHitPos, bool revCheck);		//AABB VS 球	*必須
bool isHitVS(HitAABB* atk, HitPillar* def, XMFLOAT3& resHitPos, bool revCheck);		//AABB VS 円柱
bool isHitVS(HitAABB* atk, HitRayLine* def, XMFLOAT3& resHitPos, bool revCheck);		//AABB VS 線分	*必須

bool isHitVS(HitOBB* atk, HitOBB* def, XMFLOAT3& resHitPos);			//OBB VS OBB
bool isHitVS(HitOBB* atk, HitSphere* def, XMFLOAT3& resHitPos, bool revCheck);		//OBB VS 球
bool isHitVS(HitOBB* atk, HitPillar* def, XMFLOAT3& resHitPos, bool revCheck);		//OBB VS 円柱
bool isHitVS(HitOBB* atk, HitRayLine* def, XMFLOAT3& resHitPos, bool revCheck);		//OBB VS 線分

bool isHitVS(HitSphere* atk, HitSphere* def, XMFLOAT3& resHitPos);		//球 VS 球		*必須
bool isHitVS(HitSphere* atk, HitPillar* def, XMFLOAT3& resHitPos, bool revCheck);	//球 VS 円柱
bool isHitVS(HitSphere* atk, HitRayLine* def, XMFLOAT3& resHitPos, bool revCheck);	//球 VS 線分

bool isHitVS(HitPillar* atk, HitPillar* def, XMFLOAT3& resHitPos);	//円柱 VS 円柱
bool isHitVS(HitPillar* atk, HitRayLine* def, XMFLOAT3& resHitPos, bool revCheck);	//円柱 VS 線分

bool isHitVS(HitRayLine* atk, HitRayLine* def, XMFLOAT3& resHitPos);	//線分 VS 線分	*必須

bool getVectorT(const float max, const float min, const float nml, const float pos, float& sT, float& eT);

//矩形と矩形
bool isHitVS(HitQuad* atk, HitQuad* def, XMFLOAT3& resHitPos)
{
	XMFLOAT4 myRect = atk->getHitRect();			//HitStructureが持つ判定領域を取得
	XMFLOAT4 targetRect = def->getHitRect();		//相手側のHitStructureが持つ判定領域を取得

	//X軸チェック	x:左x座標 z:右x座標
	if (targetRect.x > myRect.z || targetRect.z < myRect.x)
	{
		return false;	//X軸が重なっていない
	}

	//Y軸チェック	y:下y座標 w:上y座標
	if (targetRect.y > myRect.w || targetRect.w < myRect.y)
	{
		return false;	//Y軸が重なっていない
	}

	//x check
	XMFLOAT2 atkCenter = { (myRect.z - myRect.x) * 0.5f, (myRect.w - myRect.y) * 0.5f};
	if (atkCenter.x < targetRect.x)
	{
		resHitPos.x = targetRect.x;
	}
	else if (atkCenter.x > targetRect.z)
	{
		resHitPos.x = targetRect.z;
	}
	else
	{
		resHitPos.x = atkCenter.x;	//中点でいいんじゃないかな
	}

	//y check
	if (atkCenter.y < targetRect.y)
	{
		resHitPos.y = targetRect.y;
	}
	else if (atkCenter.y > targetRect.w)
	{
		resHitPos.y = targetRect.w;
	}
	else
	{
		resHitPos.y = atkCenter.y;	//中点でいいんじゃないかな
	}

	return true;		//どっちも重なっている
}

//矩形と円
bool isHitVS(HitQuad* atk, HitCircle* def, XMFLOAT3& resHitPos, bool revCheck)
{
	//角丸矩形と点で考えるパターン。

	XMFLOAT4 atkRect = atk->getHitRect();
	XMFLOAT2 defCent = def->getCenterPosition();

	float r = def->getRadius();	//半径

	//まずは縦長判定
	if ( (atkRect.w + r < defCent.y || atkRect.y - r > defCent.y) || (atkRect.z < defCent.x || atkRect.x > defCent.x) )
	{
		//横長判定
		if ( (atkRect.z + r < defCent.x || atkRect.x - r > defCent.x) || (atkRect.w < defCent.y || atkRect.y > defCent.y) )
		{
			//角判定＊４ どれかに入るのならヒットしている
			float x_dist, y_dist;
			float dR = def->getDoubledRadius();
			float totalDist = 0.0f;

			for (int i = 0; i < 4; i++)
			{
				switch (i)
				{
				case 0:
					//Left Bottom
					x_dist = atkRect.x - defCent.x;
					y_dist = atkRect.y - defCent.y;
					break;

				case 1:
					//Right Bottom
					x_dist = atkRect.z - defCent.x;
					y_dist = atkRect.y - defCent.y;
					break;

				case 2:
					//Left Top
					x_dist = atkRect.x - defCent.x;
					y_dist = atkRect.w - defCent.y;
					break;

				default:
					//Right Top
					x_dist = atkRect.z - defCent.x;
					y_dist = atkRect.w - defCent.y;
					break;
				}

				totalDist = x_dist * x_dist + y_dist * y_dist;
				if (totalDist <= dR)
				{
					//角に円が接触している
					switch (i)
					{
					case 0:	//lb
						resHitPos.x = atkRect.x;
						resHitPos.x = atkRect.y;
						break;

					case 1:	//rb
						resHitPos.x = atkRect.z;
						resHitPos.y = atkRect.y;
						break;

					case 2:	//lt
						resHitPos.x = atkRect.x;
						resHitPos.y = atkRect.w;
						break;

					default://rt
						resHitPos.x = atkRect.z;
						resHitPos.y = atkRect.w;
						break;
					}

					resHitPos.z = 0.0f;
					return true;
				}
			}

			//全ての判定で外れていた
			return false;
		}

	}

	//中心点と矩形の関係から矩形側の接触点を特定
	if (defCent.x < atkRect.x)
	{
		resHitPos.x = atkRect.x;
	}
	else if (defCent.x > atkRect.z)
	{
		resHitPos.x = atkRect.z;
	}
	else
	{
		resHitPos.x = defCent.x;
	}

	if (defCent.y < atkRect.y)
	{
		resHitPos.y = atkRect.y;
	}
	else if (defCent.y > atkRect.w)
	{
		resHitPos.y = atkRect.w;
	}
	else
	{
		resHitPos.y = defCent.y;
	}

	resHitPos.z = 0.0f;
	return true;
}

bool isHitVS(HitQuad* atk, HitTriangle* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitQuad* atk, HitAABB* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitQuad* atk, HitOBB* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitQuad* atk, HitSphere* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitQuad* atk, HitPillar* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitQuad* atk, HitRayLine* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

//円と円
bool isHitVS(HitCircle* atk, HitCircle* def, XMFLOAT3& resHitPos)
{
	XMFLOAT2 centerA = atk->getCenterPosition();
	XMFLOAT2 centerB = def->getCenterPosition();
	
	float xdist = centerB.x - centerA.x;	//X成分
	float ydist = centerB.y - centerA.y;	//Y成分
	
	//ベクトルの長さと半径の合計値比較(平方根計算は処理が重いのでを二乗のまま比較)
	float length = (xdist * xdist + ydist * ydist);
	if ((atk->getDoubledRadius() + def->getDoubledRadius() + 2 * atk->getRadius() * def->getRadius()) < length)
	{
		//重なっていない
		return false;
	}
	
	//どうしても平方根が必要
	length = sqrtf(length);
	XMFLOAT2 nmlVect = {xdist / length, ydist / length};

	if (length < atk->getRadius())
	{
		//atk側の中点がdefにめり込んでいる。接触点はatk上の点にする。
		resHitPos.x = centerA.x + atk->getRadius() * nmlVect.x;
		resHitPos.y = centerA.y + atk->getRadius() * nmlVect.y;
	}
	else
	{
		//def側の円周が接点
		resHitPos.x = centerB.x - def->getRadius() * nmlVect.x;
		resHitPos.y = centerB.y - def->getRadius() * nmlVect.y;
	}

	resHitPos.z = 0.0f;

	return true;		//重なっている		
}

bool isHitVS(HitCircle* atk, HitTriangle* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitCircle* atk, HitAABB* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitCircle* atk, HitOBB* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitCircle* atk, HitSphere* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitCircle* atk, HitPillar* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitCircle* atk, HitRayLine* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitTriangle* atk, HitTriangle* def, XMFLOAT3& resHitPos)
{
	return false;
}

bool isHitVS(HitTriangle* atk, HitAABB* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitTriangle* atk, HitOBB* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitTriangle* atk,  HitSphere* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitTriangle* atk,  HitPillar* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool getVectorT(const float max, const float min, const float nml, const float pos, float& sT, float& eT)
{
	//方向で接触面の順番が変わる
	if (nml < 0.0f)
	{
		//マイナス方向
		sT = (max - pos) / nml;	//方向ベクトルで割る
		eT = (min - pos) / nml;
	}
	else if (nml > 0.0f)
	{
		//プラス方向 MINが先
		sT = (min - pos) / nml;	//方向ベクトルで割る
		eT = (max - pos) / nml;
	}
	else
	{
		//傾きなし
		if (pos < min || pos > max)
		{
			//範囲に入らない
			return false;
		}
		else
		{
			sT = 0.0f;
			eT = FLT_MAX;
		}
	}

	//交差開始点０未満はベクトルの逆方向
	if (sT < 0.0f) sT = 0.0f;

	//eTがマイナスなのはベクトルが完全に反対方向、つまり軸要素が全部外。
	if (eT < 0.0f) return false;

	return true;
}

#define TRIANGLE_CHECK_MODE (1)
//Triangle Ray

#if TRIANGLE_CHECK_MODE == 0
bool isHitVS(HitTriangle* atk, HitRayLine* def, XMFLOAT3& resHitPos)
{

	//1.レイの始点ベクトル
	XMFLOAT3 stPos = def->getStart();
	XMVECTOR stVect = XMLoadFloat3(&stPos);

	//2.面の法線ベクトル
	XMVECTOR poNormal = atk->getNormal();

	//3.レイの方向ベクトル
	XMVECTOR rayNormal = def->getNormal();

	//4.レイの長さ
	float	rayLength = def->getLength();

	//Phase 1:分母になる レイの方向ベクトル・面の法線ベクトル を計算
	float dot = XMVector3Dot(rayNormal, poNormal).m128_f32[0];
	//ここでdotが０だった場合はレイと法線が直行している、つまり「面とレイが平行」
	if (dot > -0.0001f && dot < 0.0001f)	//floatが0.0は計算からは値として出てこないので、ほぼ0を0と解釈するifで判定
	{
		return false;	//レイと面が平行なので当たらない
	}

	//Phase 2:レイの始点ベクトル・面の法線ベクトル を計算
	float rayDot = XMVector3Dot(stVect, poNormal).m128_f32[0];

	//Phase 3:始点から交点までの距離を計算
	float crLen = -1.0f * rayDot / dot;

	//Phase 4:crLenがプラスで、かつ判定レイの長さより短いか確認
	if (crLen < 0.0f || crLen > rayLength)
	{
		return false;	//逆 または 遠い
	}

	//Phase 5:レイの式にcrLenを代入して交差点の座標を求める
	XMFLOAT3 position = {stPos.x + XMVectorGetX(rayNormal) * crLen, stPos.y + XMVectorGetY(rayNormal) * crLen, stPos.z + XMVectorGetZ(rayNormal) * crLen};
	
	XMFLOAT3* vertex = atk->getVertex();	//３点の座標を取得

	XMVECTOR vectAB = atk->getVectAB();		//vectABは既にあるのでこれでOK
	XMVECTOR vectBP = XMVectorSet(position.x - vertex[1].x, position.y - vertex[1].y, position.z - vertex[1].z, 0.0f);

	XMVECTOR vectBC = XMVectorSet(vertex[2].x - vertex[1].x, vertex[2].y - vertex[1].y, vertex[2].z - vertex[1].z, 0.0f);
	XMVECTOR vectCP = XMVectorSet(position.x - vertex[2].x, position.y - vertex[2].y, position.z - vertex[2].z, 0.0f);

	XMVECTOR vectCA = XMVectorSet(vertex[0].x - vertex[2].x, vertex[0].y - vertex[2].y, vertex[0].z - vertex[2].z, 0.0f);
	XMVECTOR vectAP = XMVectorSet(position.x - vertex[0].x, position.y - vertex[0].y, position.z - vertex[0].z, 0.0f);

	XMVECTOR crossABP = XMVector3Cross(vectAB, vectBP);
	XMVECTOR crossBCP = XMVector3Cross(vectBC, vectCP);
	XMVECTOR crossCAP = XMVector3Cross(vectCA, vectAP);

	//外積同士の内積を取って方向が一致しているか調べる
	if (XMVector3Dot(crossABP, crossBCP).m128_f32[0] <= 0.0f || XMVector3Dot(crossABP, crossCAP).m128_f32[0] <= 0.0f)
	{
		return false;	//逆に向いている物があったので三角形の外
	}

	//resHitPosに交点座標を書き込んで終了
	resHitPos = position;

	return true;
}

#else
//3*3正行列
float det(XMFLOAT3& a, XMFLOAT3& b, XMFLOAT3& c)
{
	float res = a.x * b.y * c.z + a.z * b.x * c.y + a.y * b.z * c.x;
	res -= a.z * b.y * c.x + a.y * b.x * c.z + a.x * b.z * c.y;
	return res;
}

bool isHitVS(HitTriangle* atk, HitRayLine* def, XMFLOAT3& resHitPos)
{
	//ここは詳細判定の処理なのでAABB部分はやらない
	XMVECTOR vectAB = atk->getVectAB();
	XMVECTOR vectAC = atk->getVectAC();

	XMFLOAT3 rayStart = def->getStart();
	XMFLOAT3 rayEnd = def->getEnd();

	//ベクトルを取り出し易いようにXMFLOAT3化
	XMFLOAT3 flA = { XMVectorGetX(vectAB), XMVectorGetY(vectAB), XMVectorGetZ(vectAB) };
	XMFLOAT3 flB = { XMVectorGetX(vectAC), XMVectorGetY(vectAC), XMVectorGetZ(vectAC) };

	//ベクトルCは -rayなので方向を逆にしておく
	XMFLOAT3 flC = { rayStart.x - rayEnd.x, rayStart.y - rayEnd.y, rayStart.z - rayEnd.z };

	//分母
	float detABC = det(flA, flB, flC);

	if (detABC < 0.0001f && detABC > -0.0001f)
	{
		//分母がほぼゼロ->Rayと三角形がほぼ直行
		return false;
	}

	detABC = 1.0f / detABC;	//先に逆数にしておく事で割り算の回数を減らす

	//三角形ABCの頂点Aの座標
	XMFLOAT3 vtxA = atk->getVertex()[0];
	//ベクトルDはSt - A
	XMFLOAT3 flD = { rayStart.x - vtxA.x, rayStart.y - vtxA.y, rayStart.z - vtxA.z };

	float u, v, t;
	u = det(flD, flB, flC) * detABC;
	v = det(flA, flD, flC) * detABC;
	t = det(flA, flB, flD) * detABC;

	if (u > 1.0f || u < 0.0f) return false;
	if (v > 1.0f || v < 0.0f) return false;
	if (t > 1.0f || t < 0.0f) return false;

	if ((u + v) > 1.0f) return false;

	//flCはRayベクトルだけど方向が逆なので引く
	resHitPos.x = rayStart.x - flC.x * t;
	resHitPos.y = rayStart.y - flC.y * t;
	resHitPos.z = rayStart.z - flC.z * t;

	return true;
}
#endif

//AABB AABB
bool isHitVS(HitAABB* atk,  HitAABB* def, XMFLOAT3& resHitPos)
{
	XMFLOAT3 atkMin, atkMax;
	XMFLOAT3 defMin, defMax;
	
	atkMin = atk->getMin();
	atkMax = atk->getMax();
	defMin = def->getMin();
	defMax = def->getMax();

	//Quadの３次元版
	if (atkMax.x < defMin.x || atkMax.y < defMin.y || atkMax.z < defMin.z ||
		defMax.x < atkMin.x || defMax.y < atkMin.y || defMax.z < atkMin.z)
		return false;

	//x check
	XMFLOAT3 atkCenter = atk->getCenter();
	if (atkCenter.x < defMin.x)
	{
		resHitPos.x = defMin.x;
	}
	else if (atkCenter.x > defMax.x)
	{
		resHitPos.x = defMax.x;
	}
	else
	{
		resHitPos.x = atkCenter.x;	//中点でいいんじゃないかな
	}

	//y check
	if (atkCenter.y < defMin.y)
	{
		resHitPos.y = defMin.y;
	}
	else if (atkCenter.y > defMax.y)
	{
		resHitPos.y = defMax.y;
	}
	else
	{
		resHitPos.y = atkCenter.y;	//中点でいいんじゃないかな
	}

	//z check
	if (atkCenter.z < defMin.z)
	{
		resHitPos.z = defMin.z;
	}
	else if (atkCenter.z > defMax.z)
	{
		resHitPos.z = defMax.z;
	}
	else
	{
		resHitPos.z = atkCenter.z;	//中点でいいんじゃないかな
	}


	return true;
}

bool isHitVS( HitAABB* atk,  HitOBB* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

//AABBと球
bool isHitVS( HitAABB* atk,  HitSphere* def, XMFLOAT3& resHitPos, bool revCheck)
{

	XMFLOAT3	atkMin = atk->getMin();
	XMFLOAT3	atkMax = atk->getMax();

	XMFLOAT3	center = def->getCenter();

	XMFLOAT3	vect = {};						//接点へのベクトル

	float		dR = def->getDoubleRadius();	//また距離なので2乗値にしておく

	float		len = 0.0f;						//合計距離値
	float		dist = 0.0f;					//計算用各軸の距離

	//X軸
	if (center.x < atkMin.x)
	{
		dist = atkMin.x - center.x;
	}
	else if (center.x > atkMax.x)
	{
		dist = atkMax.x - center.x;
	}

	len += dist * dist;					//X軸上の距離を加算。2乗を忘れずに
	vect.x = dist;
	dist = 0.0f;

	//Y軸
	if (center.y < atkMin.y)
	{
		dist = atkMin.y - center.y;
	}
	else if (center.y > atkMax.y)
	{
		dist = atkMax.y - center.y;
	}

	len += dist * dist;					//Y軸上の距離を加算。
	vect.y = dist;
	dist = 0.0f;

	//Z軸
	if (center.z < atkMin.z)
	{
		dist = atkMin.z - center.z;
	}
	else if (center.z > atkMax.z)
	{
		dist = atkMax.z - center.z;
	}

	len += dist * dist;					//Z軸上の距離を加算。
	vect.z = dist;

	if (len > dR) return false;

	//接触点算出
	resHitPos.x = center.x + vect.x;
	resHitPos.y = center.y + vect.y;
	resHitPos.z = center.z + vect.z;

	return true;
}

bool isHitVS( HitAABB* atk,  HitPillar* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitAABB* atk, HitRayLine* def, XMFLOAT3& resHitPos, bool revCheck)
{
	//AABBの範囲と線分の値が交差しているか判定（３面スラブ判定）

	XMFLOAT3 min = atk->getMin();
	XMFLOAT3 max = atk->getMax();

	XMFLOAT3 startP = def->getStart();
	XMVECTOR lineNormal = def->getNormal();
	float length = def->getLength();
	
	//X軸を基に始点からの衝突座標点の線上の割合(sT)、突破座標点の線上の割合(eT)を算出する。
	float minEnd = FLT_MAX, maxStart = FLT_MIN;
	float sT, eT;

	for (int i = 0; i < 3; i++)
	{
		float max_v, min_v, nml_v, pos_v;

		switch (i)
		{
		case 0:
			max_v = max.x;
			min_v = min.x;
			nml_v = XMVectorGetX(lineNormal);
			pos_v = startP.x;
			break;

		case 1:
			max_v = max.y;
			min_v = min.y;
			nml_v = XMVectorGetY(lineNormal);
			pos_v = startP.y;
			break;

		default:
			max_v = max.z;
			min_v = min.z;
			nml_v = XMVectorGetZ(lineNormal);
			pos_v = startP.z;
			break;
		}

		if (!getVectorT(max_v, min_v, nml_v, pos_v, sT, eT)) return false;

		//最も大きい接触点
		if (maxStart < sT) maxStart = sT;

		//最も小さい突破点
		if (minEnd > eT) minEnd = eT;

		//突破点が接触点より大きくなったら領域が交差していない
		//例外は真っ平の面
		if (maxStart > minEnd) return false;
	}

	if (length < maxStart)
		return false;	//ベクトルの長さを越えていたら当たっていない

	//交差点判明
	resHitPos.x = startP.x + XMVectorGetX(lineNormal) * maxStart;
	resHitPos.y = startP.y + XMVectorGetY(lineNormal) * maxStart;
	resHitPos.z = startP.z + XMVectorGetZ(lineNormal) * maxStart;

	return true;
}

bool isHitVS( HitOBB* atk,  HitOBB* def, XMFLOAT3& resHitPos)
{
	return false;
}

bool isHitVS( HitOBB* atk,  HitSphere* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS( HitOBB* atk,  HitPillar* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitOBB* atk, HitRayLine* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

//球と球
bool isHitVS( HitSphere* atk,  HitSphere* def, XMFLOAT3& resHitPos)
{
	float atk_r = atk->getRadius();
	float def_r = def->getRadius();

	XMFLOAT3 aC = atk->getCenter();
	XMFLOAT3 dC = def->getCenter();

	float dR = atk->getDoubleRadius() + def->getDoubleRadius() + 2.0f * atk_r * def_r;

	float dist_x, dist_y, dist_z;
	dist_x = dC.x - aC.x;
	dist_y = dC.y - aC.y;
	dist_z = dC.z - aC.z;

	float length = dist_x * dist_x + dist_y * dist_y + dist_z * dist_z;
	if (dR < length) return false;

	//接触点計算
	//中点間ベクトルの長さを取得。
	length = sqrtf(length);

	//まず単位化した方向ベクトル
	XMFLOAT3 nmlVect = {dist_x / length, dist_y / length, dist_z / length};

	//中点間ベクトル上にatkかdefかの境界がある

	if (length < def_r)
	{
		//atkの中点がdef側に埋まっている
		//atkの球面を接触点にする
		resHitPos.x = aC.x + atk_r * nmlVect.x;
		resHitPos.y = aC.y + atk_r * nmlVect.y;
		resHitPos.z = aC.z + atk_r * nmlVect.z;
	}
	else
	{
		//defの中点がatk側に埋まっている または双方の中点が十分に離れている
		//defの球面を接触点にする
		resHitPos.x = dC.x - def_r * nmlVect.x;
		resHitPos.y = dC.y - def_r * nmlVect.y;
		resHitPos.z = dC.z - def_r * nmlVect.z;
	}

	return true;
}

bool isHitVS( HitSphere* atk,  HitPillar* def, XMFLOAT3& resHitPos, bool revCheck)
{
	HitCircle aC;

	HitCircle* pDC = def->getCircle();
	HitQuad dQ;

	XMFLOAT3 circlePos;
	XMFLOAT3 sphereCenter = atk->getCenter();

	//ZX平面　円判定
	aC.setRadius(atk->getRadius());
	aC.setCenter(sphereCenter.x, sphereCenter.z);

	if (!isHitVS(&aC, pDC, circlePos))
		return false;

	//XY ZY 平面 円VS矩形判定
	aC.setCenter(sphereCenter.x, sphereCenter.y);

	float defTop = def->getTop();
	float defBottom = def->getBottom();
	float defHeight = defTop - defBottom;
	float defCenter = defBottom + defHeight * 0.5f;
	dQ.setHitSize(pDC->getRadius() * 2.0f, defHeight);
	dQ.setPosition(pDC->getCenterPosition().x, defCenter);

	XMFLOAT3 CQPos;
	if (!isHitVS(&dQ, &aC, CQPos, !revCheck))
		return false;

	aC.setCenter(sphereCenter.z, sphereCenter.y);

	//PillarのCircleは中心座標がX,Z
	dQ.setPosition(pDC->getCenterPosition().y, defCenter);

	if (!isHitVS(&dQ, &aC, CQPos, !revCheck))
		return false;

	resHitPos = circlePos;
	resHitPos.y = CQPos.y;

	return true;
}

bool isHitVS(HitSphere* atk, HitRayLine* def, XMFLOAT3& resHitPos, bool revCheck)
{
	// J K rを計算
	XMFLOAT3 lStart = def->getStart();
	XMFLOAT3 lEnd = def->getEnd();
	XMFLOAT3 spCenter = atk->getCenter();

	XMFLOAT3 vJ = { lStart.x - spCenter.x, lStart.y - spCenter.y, lStart.z - spCenter.z };
	XMFLOAT3 vK = { lEnd.x - lStart.x, lEnd.y - lStart.y, lEnd.z - lStart.z };

	float r = atk->getRadius() + def->getSize(); //Line側の半径はgetSizeメソッド

	// a b cを計算
	float a = vK.x * vK.x + vK.y * vK.y + vK.z * vK.z;	 //K ** 2 は内積
	float b = 2 * (vJ.x * vK.x + vJ.y * vK.y + vJ.z * vK.z);	 //2JK
	float c = (vJ.x * vJ.x + vJ.y * vJ.y + vJ.z * vJ.z) - r * r; //J**2 - r**2

	// b**2 - 4acを計算。ここで完全なハズレは判定出来る。
	float b24ac = b * b - 4 * a * c;

	if (b24ac < 0)
	{
		return false; //tが虚数解になるためハズレ。
	}

	//floatなので完全な0は算出し辛いので放置。0以上で計算する。
	// ｔを計算。+-二種類があるので注意。
	float sqB24ac = sqrtf(b24ac);	//平方根取得
	float tP = (-b + sqB24ac) / (2 * a);
	float tM = (-b - sqB24ac) / (2 * a);

	//tMの範囲が0.0～1.0
	if (tM >= 0.0f && tM <= 1.0f)
	{
		//貫通ortM側だけヒット
		resHitPos.x = lStart.x + vK.x * tM;
		resHitPos.y = lStart.y + vK.y * tM;
		resHitPos.z = lStart.z + vK.z * tM;
		return true;
	}
	//tPの範囲が0.0～1.0
	else if (tP >= 0.0f && tP <= 1.0f)
	{
		//tP側だけヒット
		resHitPos.x = lStart.x + vK.x * tP;
		resHitPos.y = lStart.y + vK.y * tP;
		resHitPos.z = lStart.z + vK.z * tP;
		return true;
	} //下にelseがあるので注意。
	else
	{
		//内包orハズレ
		if (tM < 0.0f && tP > 1.0f)
		{
			//内包。線分の中点をヒットにしておく
			resHitPos.x = lStart.x + vK.x * 0.5f;
			resHitPos.y = lStart.y + vK.y * 0.5f;
			resHitPos.z = lStart.z + vK.z * 0.5f;

			return true;
		}
	}

	return false;
}

bool isHitVS( HitPillar* atk,  HitPillar* def, XMFLOAT3& resHitPos)
{
	//3: Pillar VS Pillar

	//XZチェック
	if (!isHitVS(atk->getCircle(), def->getCircle(), resHitPos))
	{
		return false;
	}

	//高さチェック
	if (atk->getBottom() > def->getTop() || atk->getTop() < def->getBottom())
		return false;

	//XZ判定でresHitPosにXYにXZ値が入っているのでY値をZに移動
	resHitPos.z = resHitPos.y;

	//接触Y値
	if (atk->getTop() < def->getTop() && atk->getBottom() > def->getBottom())
	{
		//完全にdef側にatkが入っている
		resHitPos.y = atk->getCenter();
	}
	else if (def->getTop() < atk->getTop() && def->getBottom() > atk->getBottom())
	{
		//完全にatk側にdefが入っている
		resHitPos.y = def->getCenter();
	}
	else
	{
		//atkとdefはズレているので、def側の底辺または天辺の値をヒット点にしたい
		if (atk->getBottom() > def->getBottom())
		{
			//defの底辺は抜けている
			resHitPos.y = def->getTop();
		}
		else
		{
			//defの天辺は抜けている
			resHitPos.y = def->getBottom();
		}
	}

	return true;
	//3: ここまで
}

bool isHitVS(HitPillar* atk, HitRayLine* def, XMFLOAT3& resHitPos, bool revCheck)
{
	return false;
}

bool isHitVS(HitRayLine* atk, HitRayLine* def, XMFLOAT3& resHitPos)
{
	return false;
}

//判定データ（HitStructure*）保存領域をメモリ的に削除してからクリア
void HitManager::flushHitList(std::list<HitStructure*>* p_hitlist)
{
	std::for_each(p_hitlist->begin(), p_hitlist->end(),
		[](HitStructure* hitstr)
		{
			delete(hitstr);	//スマートポインタ使えば不要
		}
	);

	p_hitlist->clear();
}

void HitManager::initHitList(UINT hitTypes)
{
	m_numOfHitTypes = hitTypes;

	m_hitArray.clear();

	for (UINT i = 0; i < m_numOfHitTypes; i++)
	{
		m_hitArray.push_back( std::make_unique<list<HitStructure*>>() );
	}
}

void HitManager::setHitOrder(UINT atk, UINT def)
{
	if (atk != def && atk < m_numOfHitTypes && def < m_numOfHitTypes)
	{
		m_hitOrders.push_back({atk, def});
	}
}

void HitManager::refreshHitSystem()	//MyHitSystemの持つ判定データ保存領域を一斉クリア
{
	//unique_ptrだから参照にしないとエラー
	for_each(m_hitArray.begin(), m_hitArray.end(), [this](unique_ptr<list<HitStructure*>>& u_list) {
		flushHitList(u_list.get());
	});
}

//矩形判定登録メソッド
void HitManager::setHitArea(GameComponent* cmp, HitAreaBase* box)
{
	UINT hitType = box->getHitType();
	if (hitType < m_numOfHitTypes)
	{
		HitStructure* hit = new HitStructure(cmp, box);	//hit構造体作成。
		unique_ptr<list<HitStructure*>>& u_list = m_hitArray[hitType];
		u_list->push_back(hit);
	}
}

GameComponent* HitManager::HitStructure::getGameComponent()	//このHitStructureが持っているGameObjectを渡す
{
	return m_pGameComponent;
}

HitAreaBase* HitManager::HitStructure::getHitArea()	//このHitStructureが持っているHitBoxClassを渡す
{
	return m_pHitArea;
}

void HitManager::hitFrameAction()
{
	//ヒットチェック setHitOrder順に実行されるのでシールド判定が先に来るように設定する
	for_each(m_hitOrders.begin(), m_hitOrders.end(),

		[this](DirectX::XMUINT2& order)
		{
			unique_ptr<list<HitStructure*>>& atk = m_hitArray[order.x];

			for_each(atk->begin(), atk->end(),

				[this, order](HitStructure* p_plHit)
				{
					unique_ptr<list<HitStructure*>>& def = m_hitArray[order.y];

					for_each(def->begin(), def->end(),
						[this, p_plHit](HitStructure* p_enHit)
						{
							HitAreaBase* atkHit = nullptr;
							HitAreaBase* defHit = nullptr;

							if (p_plHit->getHitArea()->getHitShape() <= p_enHit->getHitArea()->getHitShape())
							{
								atkHit = p_plHit->getHitArea();
								defHit = p_enHit->getHitArea();
							}
							else
							{
								atkHit = p_enHit->getHitArea();
								defHit = p_plHit->getHitArea();
							}

							if (isHit(atkHit, defHit))
							{
								//ヒットしたら両方のGameObjectに通知
								p_enHit->getGameComponent()->hitReaction(p_plHit->getGameComponent()->getGameObject(), p_plHit->getHitArea());
								p_plHit->getGameComponent()->hitReaction(p_enHit->getGameComponent()->getGameObject(), p_enHit->getHitArea());
							}
						}
					);
				}
			);
		}

	);
}

bool HitManager::isHit(HitAreaBase* atkHit, HitAreaBase* defHit)
{
	bool hitcheck = false;

	//最後のヒット位置チェックでどちらからのヒット座標を出す
	bool revCheck = (atkHit->getHitShape() > defHit->getHitShape());

	if (revCheck)
	{
		//defとatkを逆にしておく（メソッドの数を減らすため）
		HitAreaBase* tempHit = defHit;
		defHit = atkHit;
		atkHit = tempHit;
	}

	switch (atkHit->getHitShape())
	{
	case HIT_SHAPE::HS_QUAD:
		switch (defHit->getHitShape())
		{
		case HIT_SHAPE::HS_QUAD:
			hitcheck = isHitVS(static_cast<HitQuad*>(atkHit), static_cast<HitQuad*>(defHit), m_lastHitPos);
			break;
		case HIT_SHAPE::HS_CIRCLE:
			hitcheck = isHitVS(static_cast<HitQuad*>(atkHit), static_cast<HitCircle*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_TRIANGLE:
			hitcheck = isHitVS(static_cast<HitQuad*>(atkHit), static_cast<HitTriangle*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_AABB:
			hitcheck = isHitVS(static_cast<HitQuad*>(atkHit), static_cast<HitAABB*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_OBB:
			hitcheck = isHitVS(static_cast<HitQuad*>(atkHit), static_cast<HitOBB*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_SPHERE:
			hitcheck = isHitVS(static_cast<HitQuad*>(atkHit), static_cast<HitSphere*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_PILLAR:
			hitcheck = isHitVS(static_cast<HitQuad*>(atkHit), static_cast<HitPillar*>(defHit), m_lastHitPos, revCheck);
			break;

		case HIT_SHAPE::HS_LINE:
			hitcheck = isHitVS(static_cast<HitQuad*>(atkHit), static_cast<HitRayLine*>(defHit), m_lastHitPos, revCheck);
			break;
		}
		break;

	case HIT_SHAPE::HS_CIRCLE:
		switch (defHit->getHitShape())
		{
		case HIT_SHAPE::HS_CIRCLE:
			hitcheck = isHitVS(static_cast<HitCircle*>(atkHit), static_cast<HitCircle*>(defHit), m_lastHitPos);
			break;
		case HIT_SHAPE::HS_TRIANGLE:
			hitcheck = isHitVS(static_cast<HitCircle*>(atkHit), static_cast<HitTriangle*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_AABB:
			hitcheck = isHitVS(static_cast<HitCircle*>(atkHit), static_cast<HitAABB*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_OBB:
			hitcheck = isHitVS(static_cast<HitCircle*>(atkHit), static_cast<HitOBB*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_SPHERE:
			hitcheck = isHitVS(static_cast<HitCircle*>(atkHit), static_cast<HitSphere*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_PILLAR:
			hitcheck = isHitVS(static_cast<HitCircle*>(atkHit), static_cast<HitPillar*>(defHit), m_lastHitPos, revCheck);
			break;

		case HIT_SHAPE::HS_LINE:
			hitcheck = isHitVS(static_cast<HitCircle*>(atkHit), static_cast<HitRayLine*>(defHit), m_lastHitPos, revCheck);
			break;
		}
		break;

	case HIT_SHAPE::HS_TRIANGLE:
		switch (defHit->getHitShape())
		{
		case HIT_SHAPE::HS_TRIANGLE:
			hitcheck = isHitVS(static_cast<HitTriangle*>(atkHit), static_cast<HitTriangle*>(defHit), m_lastHitPos);
			break;
		case HIT_SHAPE::HS_AABB:
			hitcheck = isHitVS(static_cast<HitTriangle*>(atkHit), static_cast<HitAABB*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_OBB:
			hitcheck = isHitVS(static_cast<HitTriangle*>(atkHit), static_cast<HitOBB*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_SPHERE:
			hitcheck = isHitVS(static_cast<HitTriangle*>(atkHit), static_cast<HitSphere*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_PILLAR:
			hitcheck = isHitVS(static_cast<HitTriangle*>(atkHit), static_cast<HitPillar*>(defHit), m_lastHitPos, revCheck);
			break;

		case HIT_SHAPE::HS_LINE:
			hitcheck = isHitVS(static_cast<HitTriangle*>(atkHit), static_cast<HitRayLine*>(defHit), m_lastHitPos);
			break;
		}
		break;

	case HIT_SHAPE::HS_AABB:
		switch (defHit->getHitShape())
		{
		case HIT_SHAPE::HS_AABB:
			hitcheck = isHitVS(static_cast<HitAABB*>(atkHit), static_cast<HitAABB*>(defHit), m_lastHitPos);
			break;
		case HIT_SHAPE::HS_OBB:
			hitcheck = isHitVS(static_cast<HitAABB*>(atkHit), static_cast<HitOBB*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_SPHERE:
			hitcheck = isHitVS(static_cast<HitAABB*>(atkHit), static_cast<HitSphere*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_PILLAR:
			hitcheck = isHitVS(static_cast<HitAABB*>(atkHit), static_cast<HitPillar*>(defHit), m_lastHitPos, revCheck);
			break;

		case HIT_SHAPE::HS_LINE:
			hitcheck = isHitVS(static_cast<HitAABB*>(atkHit), static_cast<HitRayLine*>(defHit), m_lastHitPos, revCheck);
			break;
		}
		break;

	case HIT_SHAPE::HS_OBB:
		switch (defHit->getHitShape())
		{
		case HIT_SHAPE::HS_OBB:
			hitcheck = isHitVS(static_cast<HitOBB*>(atkHit), static_cast<HitOBB*>(defHit), m_lastHitPos);
			break;
		case HIT_SHAPE::HS_SPHERE:
			hitcheck = isHitVS(static_cast<HitOBB*>(atkHit), static_cast<HitSphere*>(defHit), m_lastHitPos, revCheck);
			break;
		case HIT_SHAPE::HS_PILLAR:
			hitcheck = isHitVS(static_cast<HitOBB*>(atkHit), static_cast<HitPillar*>(defHit), m_lastHitPos, revCheck);
			break;

		case HIT_SHAPE::HS_LINE:
			hitcheck = isHitVS(static_cast<HitOBB*>(atkHit), static_cast<HitRayLine*>(defHit), m_lastHitPos, revCheck);
			break;
		}
		break;

	case HIT_SHAPE::HS_SPHERE:
		switch (defHit->getHitShape())
		{
		case HIT_SHAPE::HS_SPHERE:
			hitcheck = isHitVS(static_cast<HitSphere*>(atkHit), static_cast<HitSphere*>(defHit), m_lastHitPos);
			break;
		case HIT_SHAPE::HS_PILLAR:
			hitcheck = isHitVS(static_cast<HitSphere*>(atkHit), static_cast<HitPillar*>(defHit), m_lastHitPos, revCheck);
			break;

		case HIT_SHAPE::HS_LINE:
			hitcheck = isHitVS(static_cast<HitSphere*>(atkHit), static_cast<HitRayLine*>(defHit), m_lastHitPos, revCheck);
			break;
		}
		break;

	case HIT_SHAPE::HS_PILLAR:
		switch (defHit->getHitShape())
		{
		case HIT_SHAPE::HS_PILLAR:
			hitcheck = isHitVS(static_cast<HitPillar*>(atkHit), static_cast<HitPillar*>(defHit), m_lastHitPos);
			break;

		case HIT_SHAPE::HS_LINE:
			hitcheck = isHitVS(static_cast<HitPillar*>(atkHit), static_cast<HitRayLine*>(defHit), m_lastHitPos, revCheck);
			break;
		}
		break;

	case HIT_SHAPE::HS_LINE:
		switch (defHit->getHitShape())
		{
		case HIT_SHAPE::HS_LINE:
			hitcheck = isHitVS(static_cast<HitRayLine*>(atkHit), static_cast<HitRayLine*>(defHit), m_lastHitPos);
			break;
		}
		break;
	}

	return hitcheck;
}

