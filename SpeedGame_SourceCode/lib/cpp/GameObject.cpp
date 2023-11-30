#include "GameObject.h"
#include <MyAccessHub.h>
//#include <algorithm>

void GameObject::init()
{
	//初期状態は特に何もなし。必要なクラスはoverride
}

bool GameObject::action()	//フレーム処理
{
	list<GameComponent*> deleteComponents;	//削除リスト
	//deleteComponents.clear();				//ローカルならクリアなくても良いけどメンバなら必要なので注意。

	if (!isEnable()) return true;			//Enableでない場合はtrueを返して終了

	for (auto comp : components)//componentsの最初から最後まで
	{
		if (comp->isActive() && !comp->frameAction())	//中身のframeActionを実行
		{
			deleteComponents.push_back(comp);
		}
	}

	//終了コンポーネント削除モード
	if (!deleteComponents.empty())
	{
		for (auto deleteComp : deleteComponents)			//deleteComponentsの最初から最後まで
		{
			removeComponent(deleteComp);					//この中でfinishActionも呼ばれているので問題なく削除出来る。
		}

		deleteComponents.clear();		//削除リスト消去

		if (components.empty())			//コンポーネントがない
		{
			return false;				//オブジェクトの削除
		}
	}

	return true;						//次フレームもオブジェクト処理継続
}

//完全終了処理
void GameObject::cleanupGameObject()
{
	for (auto comp : components)
	{
		comp->finishAction();	//中身のfinishActionを実行
		delete(comp);			//配列自体は壊していないのでdeleteは出来る
	}

	components.clear();			//componentsの中身を全て削除

	//======Change Scene
	//Deleter実行
	if (m_pDeleter != nullptr)
	{
		m_pDeleter->ExecuteDeleter(this);
	}
	//======Change Scene END
}
