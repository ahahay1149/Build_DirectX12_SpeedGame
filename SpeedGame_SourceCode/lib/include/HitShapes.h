#pragma once
#include <Windows.h>
#include <DirectXMath.h>

using namespace DirectX;

enum class HIT_SHAPE
{
	HS_QUAD,		//2D 矩形
	HS_CIRCLE,		//2D 円
	HS_TRIANGLE,	//3D 三角形
	HS_AABB,		//3D AABB
	HS_OBB,			//3D OBB
	HS_SPHERE,		//3D 球体
	HS_PILLAR,		//3D 円柱
	HS_LINE,		//3D 線
};

class HitAreaBase
{
protected:
	HIT_SHAPE	m_hitShape;					//判定の形（計算式が変わる）
	UINT		m_hitType;					//判定の種類
	int			m_hitPower;					//判定パワー

public:
	virtual void setAttackType(UINT area, int pow)	//判定の属性とパワーを設定
	{
		m_hitType = area;
		m_hitPower = pow;
	}

	HIT_SHAPE getHitShape()
	{
		return m_hitShape;
	}

	UINT getHitType()						//判定タイプの取得
	{
		return m_hitType;
	}

	int getHitPower()						//判定攻撃力の取得
	{
		return m_hitPower;
	}
};

class HitQuad : public HitAreaBase
{
private:
	XMFLOAT2		m_hitCenter;				//判定中央座標
	XMFLOAT2		m_hitSize;					//判定の大きさ
	XMFLOAT4		m_hitRect;					//判定の矩形（計算用）

	void updateHitRect();						//判定用矩形エリアの更新 中心座標、またはサイズの変更があると再計算

public:
	HitQuad ()
	{
		m_hitShape = HIT_SHAPE::HS_QUAD;
	}

	void setHitSize(float width, float height)	//判定サイズ設定
	{
		m_hitSize.x = width;
		m_hitSize.y = height;
		updateHitRect();						//判定エリア更新
	}

	void setPosition(float x, float y)			//判定中央座標設定
	{
		m_hitCenter.x = x;
		m_hitCenter.y = y;
		updateHitRect();						//判定エリア更新
	}

	XMFLOAT4 getHitRect()						//判定領域の取得
	{
		return m_hitRect;
	}
};

//==========Circle=============
class HitCircle : public HitAreaBase
{
private:
	float radius;							//半径
	XMFLOAT2 center;						//中点
	float doubledR;							//半径の2乗

public:
	HitCircle()
	{
		m_hitShape = HIT_SHAPE::HS_CIRCLE;
	}

	void setCenter(float x, float y)		//中点設定
	{
		center.x = x;
		center.y = y;
	}

	void setRadius(float r)					//半径設定
	{
		radius = r;
		doubledR = r * r;					//平方根は毎回計算させない
	}

	float getRadius()						//半径取得　実は円同士の判定計算では不要
	{
		return radius;
	}

	float getDoubledRadius()				//判定計算で必要なのはこっち
	{
		return doubledR;
	}

	XMFLOAT2 getCenterPosition()
	{
		return center;
	}
};
//==========Circle=============

class HitTriangle : public HitAreaBase
{
private:
	XMFLOAT3	m_vertexes[3];
	XMVECTOR	m_normal;

	XMVECTOR	m_vectAB;
	XMVECTOR	m_vectAC;

public:
	HitTriangle()
	{
		m_hitShape = HIT_SHAPE::HS_TRIANGLE;
	}

	void setTriangle(const XMFLOAT3& a, const XMFLOAT3& b, const XMFLOAT3& c);

	XMFLOAT3* getVertex()
	{
		return m_vertexes;
	}

	XMVECTOR getNormal()
	{
		return m_normal;
	}

	XMVECTOR getVectAB()
	{
		return m_vectAB;
	}

	XMVECTOR getVectAC()
	{
		return m_vectAC;
	}
};

//Axis Aligbed bounding box（軸並行バウンディングボックス）
class HitAABB : public HitAreaBase
{
private:
	XMFLOAT3 m_min;
	XMFLOAT3 m_max;

	XMFLOAT3 m_center;
	XMFLOAT3 m_halfSize;

	void updateAABB();

public:
	HitAABB()
	{
		m_hitShape = HIT_SHAPE::HS_AABB;
	}

	void setAABB(const XMFLOAT3& center, const XMFLOAT3& size);
	void setAABBMinMax(const XMFLOAT3& min, const XMFLOAT3& max);
	void setCenter(const XMFLOAT3& center);
	void setSize(const XMFLOAT3& size);

	XMFLOAT3 getMin()
	{
		return m_min;
	}

	XMFLOAT3 getMax()
	{
		return m_max;
	}

	XMFLOAT3 getCenter()
	{
		return m_center;
	}

	XMFLOAT3 getHalfSize()
	{
		return m_halfSize;
	}
};

class HitOBB : public HitAreaBase
{
public:
	HitOBB()
	{
		m_hitShape = HIT_SHAPE::HS_OBB;
	}
};

class HitSphere : public HitAreaBase
{
private:
	XMFLOAT3	m_center;
	float		m_radius;
	float		m_doubleRadius;

public:
	HitSphere()
	{
		m_hitShape = HIT_SHAPE::HS_SPHERE;
	}

	void setSphereMinMax(const XMFLOAT3& min, const XMFLOAT3& max);

	void setCenter(float x, float y, float z)
	{
		m_center.x = x;
		m_center.y = y;
		m_center.z = z;
	}

	void setRadius(float r)
	{
		m_radius = r;
		m_doubleRadius = r * r;
	}

	XMFLOAT3 getCenter()
	{
		return m_center;
	}

	float getRadius()
	{
		return m_radius;
	}

	float getDoubleRadius()
	{
		return m_doubleRadius;
	}
};

class HitPillar : public HitAreaBase
{
	//1: privateメンバー
private:
	float m_height;		//高さ
	XMFLOAT3 m_center;	//中心座標
	HitCircle m_circle;	//円判定（XZ座標＋半径）

	float m_top;		//天辺Y
	float m_btm;		//底辺Y

	void updatePillar()
	{
		//中心座標と高さから天辺と底辺を計算
		m_btm = m_center.y - m_height * 0.5f;
		m_top = m_center.y + m_height * 0.5f;
	}
	//1: ここまで

public:
	HitPillar()
	{
		m_hitShape = HIT_SHAPE::HS_PILLAR;
	}

	//2:アクセサ

	//高さ設定
	void setHeight(float height)
	{
		m_height = height;
		updatePillar();
	}

	//中心点設定
	void setCenter(float x, float y, float z)
	{
		m_center.x = x;
		m_center.y = y;
		m_center.z = z;

		m_circle.setCenter(x, z);	//Circleは２次元でXZ平面
		updatePillar();
	}

	//半径設定
	void setRadius(float radius)
	{
		m_circle.setRadius(radius);
	}

	//円判定取得
	HitCircle* getCircle()
	{
		return &m_circle;
	}

	//底辺Y取得
	float getBottom()
	{
		return m_btm;
	}

	//天辺Y取得
	float getTop()
	{
		return m_top;
	}

	//中心点Y取得
	float getCenter()
	{
		return m_center.y;
	}
	//2:ここまで

};

class HitRayLine : public HitAreaBase
{
private:
	XMFLOAT3	m_start;
	XMFLOAT3	m_end;
	XMVECTOR	m_normal;
	float		m_length;
	float		m_radius;	//断面の半径

public:
	HitRayLine()
	{
		m_hitShape = HIT_SHAPE::HS_LINE;
	}

	void setLine(const XMFLOAT3& start, const XMFLOAT3& end, float radius);

	XMFLOAT3 getStart()
	{
		return m_start;
	}

	XMFLOAT3 getEnd()
	{
		return m_end;
	}

	XMVECTOR getNormal()
	{
		return m_normal;
	}

	float getLength()
	{
		return m_length;
	}

	float getSize()
	{
		return m_radius;
	}

	//=====動く地形対応
	static void GetTransformedLine(HitRayLine& ray, XMMATRIX& mat);
	//=====動く地形対応 END

};