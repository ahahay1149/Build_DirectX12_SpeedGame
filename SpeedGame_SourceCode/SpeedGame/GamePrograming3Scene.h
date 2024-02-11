#pragma once

#include <memory>
#include <GameObject.h>
#include <SceneController.h>

#include <unordered_map>

//地形判定対応
#include "TerrainComponent.h"

#include "GameManager.h"
#include "ImguiManager.h"

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

	//再生を行っている音のidを格納する
	int playingMusic[3];

	//シーンを跨いだタイムのデータを保存する
	float clearCount = 0;
	float bestScore = 0;

	std::unique_ptr<GameObject> m_systemObject;

	//GameManager
	std::unique_ptr<GameObject> m_gameManagerObject;
	GameManager* m_gameManager;

	//ImguiManager
	std::unique_ptr<GameObject> m_imguiManagerObject;
	ImguiManager* m_imguiManager;

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

	//===前のシーン情報を保持する
	UINT m_preScene;

	//各ステージの共通処理
	HRESULT InGameSceneBase();

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

	//現在のシーン識別情報を送る
	UINT getCurrentScene()
	{
		return m_scene;
	}

	//1つ前のシーン識別情報を送る
	UINT getPreviousScene()
	{
		return m_preScene;
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
