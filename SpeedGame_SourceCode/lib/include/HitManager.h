#pragma once
#include <Windows.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>
#include <list>
#include "GameObject.h"

#include "HitShapes.h"

using namespace DirectX;
using namespace std;

class GameComponent;

class HitManager
{
private:
    class HitStructure	//クラス内クラス
    {
    private:
		HitAreaBase* m_pHitArea;		            //判定領域データ
        GameComponent* m_pGameComponent;            //判定を設定したGameObject
    public:
        HitStructure(GameComponent* cmp, HitAreaBase* hitArea)
        {
            m_pHitArea = hitArea;
            m_pGameComponent = cmp;
        }

        //bool isHit(HitStructure* target);			//解説した当たり判定チェックを行うメソッド trueでヒット
        GameComponent* getGameComponent();			//判定を設定したGameObjectを取得
		HitAreaBase* getHitArea();					//登録してある判定データ本体を取得
    };

	list<DirectX::XMUINT2> m_hitOrders;
	
    UINT m_numOfHitTypes = 0;
    vector<unique_ptr<list<HitStructure*>>> m_hitArray;

    void flushHitList(list<HitStructure*>* p_hitlist);  //判定保存用listのクリア　refreshHitSystemはこれを全部に行う

    XMFLOAT3    m_lastHitPos;

public:
	void initHitList(UINT hitTypes);
	void setHitOrder(UINT atk, UINT def);

    void refreshHitSystem();                                    //ヒットシステムデータクリア　フレーム開始時に実行する。
    void setHitArea(GameComponent * cmp, HitAreaBase* box);     //当たり判定システムに判定データを登録する

    void hitFrameAction();                                      //判定一斉処理　登録順の前後で結果が変わらないようにしている

    bool isHit(HitAreaBase* atkHit, HitAreaBase* defHit);

    XMFLOAT3 getLastHitPoint()
    {
        return m_lastHitPos;
    }
};
