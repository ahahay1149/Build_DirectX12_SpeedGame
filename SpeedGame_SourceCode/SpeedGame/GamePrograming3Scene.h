#pragma once

#include <memory>
#include <GameObject.h>
#include <SceneController.h>

#include <unordered_map>

//地形判定対応
#include "TerrainComponent.h"

#include "UnityChanPlayer.h"
#include "GamePrograming3UIRender.h"
#include "ResultUIRender.h"


//=========Scene Change
class GamePrograming3Scene;

class SceneObjectDeleter : public AbstractGameObjectDeleter
{
private:
	GamePrograming3Scene* myScene;
public:
	SceneObjectDeleter(GamePrograming3Scene* scene)
	{
		myScene = scene;
	}
	// AbstractGameObjectDeleter を介して継承されました
	virtual void ExecuteDeleter(GameObject* go) override;
};
//=========Scene Change END

class GamePrograming3Scene : public SceneController
{
private:
	//各クラスのアクセス指定子
	static GamePrograming3Scene* m_Scene;
	static UnityChanPlayer* m_UnityChan;
	static GamePrograming3UIRender* m_UIRender;
	static ResultUIRender* m_ResultUIRender;

	//再生を行っている音のidを格納する
	int playingMusic[3];

	//シーンを跨いだタイムのデータを保存する
	float clearCount = 0;
	float bestScore = 0;

	std::unique_ptr<GameObject> m_systemObject;

	//====CameraFix
	GameComponent* m_keyComponent;
	std::unordered_map<std::wstring, GameComponent*> m_cameraComponents;
	//====CameraFix

	//地形判定Component。オープンワールド的に複数保持可能に
	std::vector<TerrainComponent*> m_terrains;

	//=========Scene Change
	std::list<std::unique_ptr<GameObject>> m_SceneObjects;
	std::unique_ptr<SceneObjectDeleter> m_pObjDeleter;
	void ClearSceneObjects();
	//=========Scene Change End

public:
	~GamePrograming3Scene();


	HRESULT initSceneController() override;
	HRESULT changeGameScene(UINT scene) override;

	GameComponent* getKeyComponent()
	{
		return m_keyComponent;
	}

	GameComponent* getCameraComponent(const std::wstring cameraName)
	{
		if (m_cameraComponents[cameraName] != nullptr)
		{
			return m_cameraComponents[cameraName];
		}

		return nullptr;
	}

	//Scene Class
	static void setScene(GamePrograming3Scene* Scene)
	{
		m_Scene = Scene;
	}

	static GamePrograming3Scene* getScene()
	{
		return m_Scene;
	}

	//UnityChan Class
	static void setUnityChan(UnityChanPlayer* unityChan)
	{
		m_UnityChan = unityChan;
	}

	static UnityChanPlayer* getUnityChan()
	{
		return m_UnityChan;
	}

	//InGameUI Class
	static void setUIRender(GamePrograming3UIRender* UIRender)
	{
		m_UIRender = UIRender;
	}

	static GamePrograming3UIRender* getUIRender()
	{
		return m_UIRender;
	}

	//ResultUI Class
	static void setResultUIRender(ResultUIRender* UIRender)
	{
		m_ResultUIRender = UIRender;
	}

	static ResultUIRender* getResultUIRender()
	{
		return m_ResultUIRender;
	}

	void setClearCount(float timer)
	{
		clearCount = timer;
	}

	float getClearCount()
	{
		return clearCount;
	}

	void setBestScore(float timer)
	{
		bestScore = timer;
	}

	float getBestScore()
	{
		return bestScore;
	}

	//現在のシーン番号を送る
	UINT getCurrentScene()
	{
		return m_scene;
	}

	//====CameraFix
	void RemoveCamera(GameComponent* gc);
	void RemoveCamera(std::wstring label);
	//====CameraFix End

	//=========Scene Change
	void AddSceneObject(GameObject* obj) override;
	void DeleteSceneObject(GameObject* obj) override;
	//=========Scene Change End

	//TerrainComponent取得
	TerrainComponent* getTerrainComponent(int index)
	{
		if (m_terrains.size() <= index)
			return nullptr;

		return m_terrains[index];
	}

	//地形データクリア。
	void clearTerrains()
	{
		m_terrains.clear();
	}
};
