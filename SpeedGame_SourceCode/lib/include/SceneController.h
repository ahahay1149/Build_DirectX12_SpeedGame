#pragma once

#include <Windows.h>
#include "GameObject.h"

class GameObject;

class SceneController
{
protected:
	UINT m_scene = 0;
	//=======Change Scene
	UINT m_orderSceneID;	//
	HRESULT virtual changeGameScene(UINT scene) = 0;
	//=======Change Scene END


public:
	//virtualにしているのはシーン構造を作る時にオーバライド出来るように
	//m_sceneがGAME_SCENESじゃなくてUINTなのは、このenum自体がゲームごとに変わるため
	HRESULT virtual initSceneController() = 0;
	//=======Change Scene
	void OrderNextScene(UINT scene)
	{
		m_orderSceneID = scene;
	}

	void CheckSceneOrder();
	//=======Change Scene END

	//仮想デストラクタ
	virtual ~SceneController() {};

	virtual void AddSceneObject(GameObject* obj) = 0;
	virtual void DeleteSceneObject(GameObject* obj) = 0;

};