﻿#include "GamePrograming3Scene.h"

#include "GamePrograming3Enum.h"
#include "MyAccessHub.h"
#include "KeyBindComponent.h"

#include "SpriteRenderPipeline.h"
#include "FBXCharacterData.h"
#include "StandardLightingPipeline.h"
#include "LightSettingManager.h"

//======G Buffer
#include "GBufferResetCommand.h"
//======G Buffer End

//======Edge Draw
#include "EdgeDrawPipeline.h"
//======Edge Draw End

//======Depth Shadow(Pre Pipeline)
#include "ShadowMapPipeline.h" 
//======Depth Shadow(Pre Pipeline) End

//Fbx Loadの最適化（一つのメッシュを使い回せるように処理を変更）
#include "FBXDataContainerSystem.h"

//GameObjects
#include "SkyDomeComponent.h"
#include "TerrainComponent.h"
#include "CameraComponent.h"
//Player
#include "UnityChanPlayer.h"

//Camera
#include "CameraChangerComponent.h"			//カメラ切り替え
#include "FlyingCameraController.h"			//高高度カメラ
#include "ThirdPersonCameraController.h"	//三人称カメラ

//UI
#include "TitleUIRender.h"					//タイトルUI
#include "GameOverUIRender.h"				//ゲームオーバーUI
#include "GamePrograming3UIRender.h"		//インゲームUI
#include "ResultUIRender.h"					//リザルトUI

//ゲームアプリ用AccessHub
#include "GameAccessHub.h"
//ゲームアプリ用Enum
#include "GameAppEnum.h"

//HeartItem
#include "HeartItemComponent.h"
#include "HeartItemInfo.h"

//=====動く地形
#include "MovingPlatform.h"
//=====動く地形 End

//=====Change Scene
#include "TitleSceneSample.h"
//=====Change Scene End


//=========Change Scene
void GamePrograming3Scene::AddSceneObject(GameObject* obj)
{
	MyGameEngine* engine = MyAccessHub::getMyGameEngine();
	//Deleterセット
	obj->SetGameObjectDeleter(m_pObjDeleter.get());
	m_SceneObjects.push_back(unique_ptr<GameObject>(obj));
	//engine->AddGameObject(obj);
	engine->SetGameObjectToAddQueue(obj);
}

void GamePrograming3Scene::DeleteSceneObject(GameObject* obj)
{
	auto ite = m_SceneObjects.begin();
	//objを持っている場所を探す
	for (; ite != m_SceneObjects.end(); ite++)
	{
		if (ite->get() == obj)
		{
			break;
		}
	}
	//removeがあるのでループの外で処理
	if (ite != m_SceneObjects.end())
	{
		ite->release(); //所有権放棄（deleteを他に任せる）
		m_SceneObjects.remove(*ite);
	}

}

void GamePrograming3Scene::ClearSceneObjects()
{
	if (m_systemObject == nullptr)
		return;

	MyGameEngine* engine = MyAccessHub::getMyGameEngine();
	for (auto ite = m_SceneObjects.begin(); ite != m_SceneObjects.end(); ite++)
	{
		GameObject* go = ite->get();
		ite->release(); //所有権放棄（deleteを他に任せる）
		engine->RemoveGameObject(go);

		delete(go);
	}
	m_SceneObjects.clear();
	m_terrains.clear();

}
//=========Change Scene End

GamePrograming3Scene::~GamePrograming3Scene()
{
	m_terrains.clear();
	m_SceneObjects.clear();
	m_cameraComponents.clear();
}

HRESULT GamePrograming3Scene::initSceneController()
{

	m_scene = static_cast<UINT>(GAME_SCENES::AWAKE);

	//=========Change Scene
	m_pObjDeleter = std::make_unique<SceneObjectDeleter>(this);

	HRESULT res = changeGameScene(static_cast<UINT>(GAME_SCENES::INIT));

	if (SUCCEEDED(res))
		m_orderSceneID = m_scene;

	return res;
	//=========Change Scene END

}

HRESULT GamePrograming3Scene::changeGameScene(UINT scene)
{
	HRESULT hr = S_OK;
	MyGameEngine* engine = MyAccessHub::getMyGameEngine();

	//現在のシーンと引数に設定されているシーンが異なる場合のみ実行
	if (m_scene != scene)
	{

		//=====Debug Object Clear
		if(m_imguiManager != nullptr)
			m_imguiManager->clearImguiObject(scene);
		//=====Debug Object Clear End
		
		//=====Change Scene
		ClearSceneObjects();
		//=====Change Scene End

		//前のシーン情報を登録
		m_preScene = m_scene;

		//Phase 3 Fbx管理を別の処理にまとめたため、Fbx読み込み指示がここに集中
		switch (m_scene)
		{
			case static_cast<UINT>(GAME_SCENES::IN_GAME):
			case static_cast<UINT>(GAME_SCENES::IN_GAME02):
			case static_cast<UINT>(GAME_SCENES::IN_GAME03):
			{
				FBXDataContainerSystem* fbxSys = FBXDataContainerSystem::GetInstance();
				fbxSys->DeleteModelFBX(L"SkyDome");
				fbxSys->DeleteModelFBX(L"TerrainSample");
				fbxSys->DeleteModelFBX(L"UnityChan");
				fbxSys->DeleteModelFBX(L"Platform");
				fbxSys->DeleteModelFBX(L"GoldenHeart");
				fbxSys->DeleteModelFBX(L"RedHeart");
				fbxSys->DeleteModelFBX(L"BlueHeart");

				fbxSys->DeleteAnimeFBX(L"WAIT00");
				fbxSys->DeleteAnimeFBX(L"WALK_F");
				fbxSys->DeleteAnimeFBX(L"JUMP");
			}
				break;
		}

		switch (scene)
		{
		case static_cast<UINT>(GAME_SCENES::INIT):	//ゲームシステム全体の初期化
			//テクスチャと効果音の読み込み
			{
				//=====Camera Change Phase 1
				engine->GetTextureManager()->CreateTextureFromFile(engine->GetDirect3DDevice(), L"HUDTexture", L"./Resources/textures/HUD/UITexture.png");
				//=====Camera Change Phase 1 End
				
				//=====Change Scene
				engine->GetTextureManager()->CreateTextureFromFile(engine->GetDirect3DDevice(), L"TitleTexture", L"./Resources/textures/Title/TitleSample.png");
				//=====Change Scene End
				
				//======Toon Shader
				engine->GetTextureManager()->CreateTextureFromFile(engine->GetDirect3DDevice(), L"ToneTexture", L"./Resources/textures/ToonShader/Tone.png");
				//======Edge Draw
				engine->GetTextureManager()->CreateRenderTargetTexture
				(engine->GetDirect3DDevice(), L"NormalBuffer", engine->GetWidth(), engine->GetHeight(), DXGI_FORMAT_R32G32B32A32_FLOAT);
				//======Edge Draw End
				//======Toon Shader End

				engine->GetMeshManager()->createPresetMeshData();

				//=========Pre Draw Pipeline
				PipeLineManager* pPreDrawMng = engine->GetPreDrawPipelineManager();

				//======G Buffer System
				GBufferResetCommand* gbuffPL = new GBufferResetCommand();
				pPreDrawMng->AddPipeLineObject(L"GBuffReset", gbuffPL);
				//======G Buffer System End
				
				//======Post Effect PipeLine
				EdgeDrawPipeline* edgePL = new EdgeDrawPipeline();
				engine->GetPostEffectPipelineManager()->AddPipeLineObject(L"EdgeDraw", edgePL);
				//======Post Effect PipeLine End

				//======Shadow Map Pipeline
				ShadowMapPipeline* pShadowMap = new ShadowMapPipeline();
				pShadowMap->SetStaticMeshMode(true);
				pPreDrawMng->AddPipeLineObject(L"StaticShadowMap", pShadowMap);

				pShadowMap = new ShadowMapPipeline();
				pShadowMap->SetStaticMeshMode(false);
				pPreDrawMng->AddPipeLineObject(L"SkeltalShadowMap", pShadowMap);
				//======Shadow Map Pipeline End
				
				//=========Pre Draw Pipeline End

				//==========PipeLineManager
				PipeLineManager* plMng = engine->GetPipelineManager();

				SpriteRenderPipeline* spritePL = new SpriteRenderPipeline();
				spritePL->SetSamplerMode(0);	//フィルタなし
				spritePL->SetBlendMode(0);		//カットオフのみ
				plMng->AddPipeLineObject(L"Sprite", spritePL);

				spritePL = new SpriteRenderPipeline();
				spritePL->SetSamplerMode(0);	//フィルタなし
				spritePL->SetBlendMode(1);		//アルファブレンド
				plMng->AddPipeLineObject(L"AlphaSprite", spritePL);

				//FBX
				StandardLightingPipeline* fbxPL = new StandardLightingPipeline();
				fbxPL->SetPipelineFlags(0);
				plMng->AddPipeLineObject(L"StaticFBX", fbxPL);

				fbxPL = new StandardLightingPipeline();
				fbxPL->SetPipelineFlags(StandardLightingPipeline::PIPELINE_FLAGS::SKELTAL);
				plMng->AddPipeLineObject(L"AnimationFBX", fbxPL);

				//======Lighting
				LightSettingManager* lightMng = LightSettingManager::GetInstance();
				XMFLOAT3 lightColor;
				XMFLOAT3 lightDirection;

				//Ambient
				lightColor = { 0.4f, 0.4f, 0.4f };	//1.0でライトの影響値ゼロ
				lightMng->CreateAmbientLight(L"SCENE_AMBIENT", lightColor);	//登録名はFBXCharacterData参照

				//DirectionalLight
				lightColor = { 0.5f, 0.5f, 0.5f };			//昼光色的な
				lightDirection = { -0.57f, -0.57f, 0.57f };	//左斜め下Z奥向き
				lightMng->CreateDirectionalLight(L"SCENE_DIRECTIONAL", lightColor, lightDirection);	//登録名はFBXCharacterData参照
				//======Lighting End

				//======Lambert Pipeline
				fbxPL = new StandardLightingPipeline();
				fbxPL->SetPipelineFlags(StandardLightingPipeline::PIPELINE_FLAGS::Lambert);
				plMng->AddPipeLineObject(L"StaticLambert", fbxPL);	//Staticメッシュ+Lambert

				fbxPL = new StandardLightingPipeline();
				fbxPL->SetPipelineFlags(StandardLightingPipeline::PIPELINE_FLAGS::SKELTAL | StandardLightingPipeline::PIPELINE_FLAGS::Lambert);
				plMng->AddPipeLineObject(L"SkeltalLambert", fbxPL);
				//======Lambert Pipeline End

				//======Phong Pipeline
				fbxPL = new StandardLightingPipeline();
				fbxPL->SetPipelineFlags(StandardLightingPipeline::PIPELINE_FLAGS::Lambert | StandardLightingPipeline::PIPELINE_FLAGS::Phong);
				plMng->AddPipeLineObject(L"StaticPhong", fbxPL);

				fbxPL = new StandardLightingPipeline();
				fbxPL->SetPipelineFlags(StandardLightingPipeline::PIPELINE_FLAGS::SKELTAL | StandardLightingPipeline::PIPELINE_FLAGS::Lambert | StandardLightingPipeline::PIPELINE_FLAGS::Phong);
				plMng->AddPipeLineObject(L"SkeltalPhong", fbxPL);
				//======Phong Pipeline End

				//======Blinn Phong Pipeline
				fbxPL = new StandardLightingPipeline();
				fbxPL->SetPipelineFlags(StandardLightingPipeline::PIPELINE_FLAGS::Lambert | StandardLightingPipeline::PIPELINE_FLAGS::Blinn);
				plMng->AddPipeLineObject(L"StaticBlinn", fbxPL);

				fbxPL = new StandardLightingPipeline();
				fbxPL->SetPipelineFlags(StandardLightingPipeline::PIPELINE_FLAGS::SKELTAL | StandardLightingPipeline::PIPELINE_FLAGS::Lambert | StandardLightingPipeline::PIPELINE_FLAGS::Blinn);
				plMng->AddPipeLineObject(L"SkeltalBlinn", fbxPL);
				//======Blinn Phong Pipeline End

				//======Toon Shader
				fbxPL = new StandardLightingPipeline();
				fbxPL->SetPipelineFlags(StandardLightingPipeline::PIPELINE_FLAGS::Lambert | StandardLightingPipeline::PIPELINE_FLAGS::Toon);
				plMng->AddPipeLineObject(L"StaticToon", fbxPL);

				fbxPL = new StandardLightingPipeline();
				fbxPL->SetPipelineFlags(StandardLightingPipeline::PIPELINE_FLAGS::SKELTAL | StandardLightingPipeline::PIPELINE_FLAGS::Lambert | StandardLightingPipeline::PIPELINE_FLAGS::Toon);
				plMng->AddPipeLineObject(L"SkeltalToon", fbxPL);
				//======Toon Shader End

				//==========PipeLineManager End

				HitManager* hitMng = engine->GetHitManager();

				hitMng->initHitList(7);

				//シールド判定を先にチェックしないと、シールド判定とダメージ判定を同時に行ってしまうので処理順には注意
				hitMng->setHitOrder((UINT)HIT_ORDER::HIT_PLAYER_ATTACK, (UINT)HIT_ORDER::HIT_ENEMY_SHIELD);	//プレイヤ攻撃VS敵盾
				hitMng->setHitOrder((UINT)HIT_ORDER::HIT_PLAYER_ATTACK, (UINT)HIT_ORDER::HIT_ENEMY_BODY);	//プレイヤ攻撃VS敵
				hitMng->setHitOrder((UINT)HIT_ORDER::HIT_ENEMY_ATTACK, (UINT)HIT_ORDER::HIT_PLAYER_SHIELD);	//敵攻撃VSプレイヤ盾
				hitMng->setHitOrder((UINT)HIT_ORDER::HIT_ENEMY_ATTACK, (UINT)HIT_ORDER::HIT_PLAYER_BODY);	//敵攻撃VSプレイヤ
				hitMng->setHitOrder((UINT)HIT_ORDER::HIT_PLAYER_BODY, (UINT)HIT_ORDER::HIT_ENEMY_BODY);		//プレイヤVS敵
				hitMng->setHitOrder((UINT)HIT_ORDER::HIT_PLAYER_BODY, (UINT)HIT_ORDER::HIT_ITEM);			//プレイヤVSアイテム

				SoundManager* soMng = engine->GetSoundManager();

				int soundId = 0;

				if (!soMng->loadSoundFile(L"./Resources/sounds/InGameBGM.wav", soundId))			//00 InGameBGM
					return E_FAIL;
				if (!soMng->loadSoundFile(L"./Resources/sounds/InGameBGM02.wav", soundId))			//01 InGameBGM02
					return E_FAIL;
				if (!soMng->loadSoundFile(L"./Resources/sounds/InGameBGM03.wav", soundId))			//02 InGameBGM03
					return E_FAIL;
				if (!soMng->loadSoundFile(L"./Resources/sounds/CountDownWait.wav", soundId))		//03 CountDownWait
					return E_FAIL;
				if (!soMng->loadSoundFile(L"./Resources/sounds/CountDownStart.wav", soundId))		//04 CountDownStart
					return E_FAIL;
				if (!soMng->loadSoundFile(L"./Resources/sounds/ClearStageJingle.wav", soundId))		//05 StageClearJingle
					return E_FAIL;
				if (!soMng->loadSoundFile(L"./Resources/sounds/ClearGameJingle.wav", soundId))		//06 GameClearJingle
					return E_FAIL;
				if (!soMng->loadSoundFile(L"./Resources/sounds/GameOverJingle.wav", soundId))		//07 GameOverJingle
					return E_FAIL;
				if (!soMng->loadSoundFile(L"./Resources/sounds/HeartGold.wav", soundId))			//08 GoldHeart
					return E_FAIL;
				if (!soMng->loadSoundFile(L"./Resources/sounds/HeartRed.wav", soundId))				//09 RedHeart
					return E_FAIL;
				if (!soMng->loadSoundFile(L"./Resources/sounds/HeartBlue.wav", soundId))			//10 BlueHeart
					return E_FAIL;

				//システム制御統合オブジェクト登録
				m_systemObject = make_unique<GameObject>(nullptr);
				m_keyComponent = new KeyBindComponent();
				m_systemObject->addComponent(m_keyComponent);
				engine->AddGameObject(m_systemObject.get());

				//ImguiManagerオブジェクト登録
				m_imguiManagerObject = make_unique<GameObject>(nullptr);
				m_imguiManager = new ImguiManager();
				m_imguiManagerObject->addComponent(m_imguiManager);
				engine->AddGameObject(m_imguiManagerObject.get());

				//GameManagerオブジェクト登録
				m_gameManagerObject = make_unique<GameObject>(nullptr);
				m_gameManager = new GameManager();
				m_gameManagerObject->addComponent(m_gameManager);
				GameAccessHub::setGameManager(m_gameManager);
				//ImGui Set
				m_imguiManager->setImguiObject("All_GameManager", m_gameManager, DEBUG_FLAG::Scene_All);

				engine->AddGameObject(m_gameManagerObject.get());

				engine->UploadCreatedTextures();

				//=========Change Scene
				return changeGameScene(static_cast<UINT>(GAME_SCENES::TITLE));	//タイトル画面へ
				//=========Change Scene END
			}
			break;

		case static_cast<UINT>(GAME_SCENES::TITLE):
			{
				GameObject* cameraObj = new GameObject(new CharacterData());
				TitleSceneSample* titleCamera = new TitleSceneSample();
				cameraObj->addComponent(titleCamera);

				TitleUIRender* titleRender = new TitleUIRender();
				cameraObj->addComponent(titleRender);

				titleCamera->SetBGColor(0.5f, 0.5f, 0.5f);
				titleCamera->SetFontPosition(74.0f);

				AddSceneObject(cameraObj);

				//カメラリスト追加 UIモード時用
				m_cameraComponents[L"TitleCamera"] = titleCamera;
			}
			break;

		case static_cast<UINT>(GAME_SCENES::IN_GAME):
			{
				//各ステージの共通処理
				hr = InGameSceneBase();

				//使いまわしするのでここで宣言
				GameObject* heartObj;
				HeartItemComponent* heartItem;

				//======動く地形
				FBXCharacterData* terrainFbx = new FBXCharacterData();
				terrainFbx->SetMainFBX(L"Platform");
				terrainFbx->setPosition(10.0f, 0.5f, 10.0f);

				//地形用オブジェクト作成
				GameObject* terrainObj = new GameObject(terrainFbx);
				TerrainComponent* trCom = new TerrainComponent();
				terrainObj->addComponent(trCom);

				//ImGui Set
				m_imguiManager->setImguiObject("GameScene_MovingStage", trCom,
					DEBUG_FLAG::Scene_InGame | DEBUG_FLAG::Component_Terrain | DEBUG_FLAG::Shader_Stage);

				terrainObj->addComponent(new MovingPlatform());
				AddSceneObject(terrainObj);

				//m_terrainsにTerrainComponentを登録
				m_terrains.push_back(trCom);
				//======動く地形 End

				//ハート生成
				for (int i = 0; i < HeartInfo::heartNum; i++)
				{
					FBXCharacterData* heartFbx = new FBXCharacterData();
					heartFbx->SetMainFBX(HeartType::typeMap.at(HeartInfo::heart01[i].type));
					heartFbx->setPosition(HeartInfo::heart01[i].x, HeartInfo::heart01[i].y, HeartInfo::heart01[i].z);

					heartObj = new GameObject(heartFbx);
					heartItem = new HeartItemComponent();
					heartItem->setHeartSpeed(HeartInfo::heart01[i].playerSpeed);
					heartItem->setHeartType(static_cast<int>(HeartInfo::heart01[i].type));
					heartObj->addComponent(heartItem);

					//ImGui Set
					m_imguiManager->setImguiObject("Scene01_HeartItem:" + std::to_string(i + 1), heartItem,
						DEBUG_FLAG::Scene_InGame | DEBUG_FLAG::Component_HeartItem | DEBUG_FLAG::Shader_Stage);

					AddSceneObject(heartObj);
				}

				engine->UploadCreatedTextures();

				//======MipMap
				engine->GetTextureManager()->GenerateMipMap();
				//======MipMap End
			}
			break;

		case static_cast<UINT>(GAME_SCENES::IN_GAME02):
			{
				//各ステージの共通処理
				hr = InGameSceneBase();

				//Stage02に合わせたUnityChanPlayerの位置を設定
				GameObject* gameObj = static_cast<GameComponent*>(GameAccessHub::getUnityChan())->getGameObject();
				gameObj->getCharacterData()->setPosition(Player::stagePos[1].x, Player::stagePos[1].y, Player::stagePos[1].z);

				//使いまわしするのでここで宣言
				GameObject* heartObj;
				HeartItemComponent* heartItem;

				//ハート生成
				for (int i = 0; i < HeartInfo::heartNum; i++)
				{
					FBXCharacterData* heartFbx = new FBXCharacterData();
					heartFbx->SetMainFBX(HeartType::typeMap.at(HeartInfo::heart02[i].type));
					heartFbx->setPosition(HeartInfo::heart02[i].x, HeartInfo::heart02[i].y, HeartInfo::heart02[i].z);

					heartObj = new GameObject(heartFbx);
					heartItem = new HeartItemComponent();
					heartItem->setHeartSpeed(HeartInfo::heart02[i].playerSpeed);
					heartItem->setHeartType(static_cast<int>(HeartInfo::heart02[i].type));
					heartObj->addComponent(heartItem);

					//ImGui Set
					m_imguiManager->setImguiObject("Scene02_HeartItem:" + std::to_string(i + 1), heartItem,
						DEBUG_FLAG::Scene_InGame02 | DEBUG_FLAG::Component_HeartItem | DEBUG_FLAG::Shader_Stage);

					AddSceneObject(heartObj);
				}

				engine->UploadCreatedTextures();

				//======MipMap
				engine->GetTextureManager()->GenerateMipMap();
				//======MipMap End
			}
			break;
		case static_cast<UINT>(GAME_SCENES::IN_GAME03):
			{
				//各ステージの共通処理
				hr = InGameSceneBase();

				//Stage03に合わせたUnityChanPlayerの位置を設定
				GameObject* gameObj = static_cast<GameComponent*>(GameAccessHub::getUnityChan())->getGameObject();
				gameObj->getCharacterData()->setPosition(Player::stagePos[2].x, Player::stagePos[2].y, Player::stagePos[2].z);

				//使いまわしするのでここで宣言
				GameObject* heartObj;
				HeartItemComponent* heartItem;

				//ハート生成
				for (int i = 0; i < HeartInfo::heartNum; i++)
				{
					FBXCharacterData* heartFbx = new FBXCharacterData();
					heartFbx->SetMainFBX(HeartType::typeMap.at(HeartInfo::heart03[i].type));
					heartFbx->setPosition(HeartInfo::heart03[i].x, HeartInfo::heart03[i].y, HeartInfo::heart03[i].z);

					heartObj = new GameObject(heartFbx);
					heartItem = new HeartItemComponent();
					heartItem->setHeartSpeed(HeartInfo::heart03[i].playerSpeed);
					heartItem->setHeartType(static_cast<int>(HeartInfo::heart03[i].type));
					heartObj->addComponent(heartItem);

					//ImGui Set
					m_imguiManager->setImguiObject("Scene03_HeartItem:" + std::to_string(i + 1), heartItem,
						DEBUG_FLAG::Scene_InGame03 | DEBUG_FLAG::Component_HeartItem | DEBUG_FLAG::Shader_Stage);

					AddSceneObject(heartObj);
				}

				engine->UploadCreatedTextures();

				//======MipMap
				engine->GetTextureManager()->GenerateMipMap();
				//======MipMap End
				}
			break;

		case static_cast<UINT>(GAME_SCENES::GAME_OVER):
			{
				GameObject* CameraObj = new GameObject(new CharacterData());
				TitleSceneSample* overCamera = new TitleSceneSample();
				CameraObj->addComponent(overCamera);

				GameOverUIRender* uiRender = new GameOverUIRender();
				CameraObj->addComponent(uiRender);

				overCamera->SetBGColor(0.5f, 0.5f, 0.5f);
				overCamera->SetFontPosition(200.0f);

				AddSceneObject(CameraObj);

				//カメラリスト追加 UIモード時用
				m_cameraComponents[L"TitleCamera"] = overCamera;
			}
			break;

		case static_cast<UINT>(GAME_SCENES::GAME_CLEAR):
			{
				GameObject* cameraObj = new GameObject(new CharacterData());
				TitleSceneSample* clearCamera = new TitleSceneSample();
				cameraObj->addComponent(clearCamera);

				//======Camera Change Phase 1
				ResultUIRender* uiRender = new ResultUIRender();
				cameraObj->addComponent(uiRender);

				//カメラリスト追加 UIモード時用
				m_cameraComponents[L"HUDCamera"] = uiRender;
				//======Camera Change Phase 1 End

				clearCamera->SetBGColor(0.5f, 0.5f, 0.5f);
				clearCamera->SetFontPosition(375.0f);

				AddSceneObject(cameraObj);

				//カメラリスト追加 UIモード時用
				m_cameraComponents[L"TitleCamera"] = clearCamera;
			}
			break;

		default:
			return E_FAIL;	//シーン指定がおかしい
		}

		m_scene = scene;

		//シーン変更時の処理を実行する
		m_gameManager->sendScene(scene);

		engine->WaitForGpu();	//GPU待機（テクスチャアップロード等）
	}

	return hr;
}

//======CameraChange Phase 1
void GamePrograming3Scene::RemoveCamera(GameComponent* gc)
{
	std::wstring label = L"";
	for (auto cameraPair : m_cameraComponents)
	{
		if (cameraPair.second == gc)
		{
			label = cameraPair.first;
			break;
		}
	}

	if (label != L"")
	{
		RemoveCamera(label);
	}
}

void GamePrograming3Scene::RemoveCamera(std::wstring label)
{
	if (m_cameraComponents [label] != nullptr)
	{
		m_cameraComponents.erase(label);
	}
}
//======CameraChange Phase 1 End

void SceneObjectDeleter::ExecuteDeleter(GameObject* go)
{
	myScene->DeleteSceneObject(go); //Deleterを経由すればGameObjectの削除からこれが呼べる
}

HRESULT GamePrograming3Scene::InGameSceneBase()
{
	HRESULT hr = S_OK;
	MyGameEngine* engine = MyAccessHub::getMyGameEngine();

	//Phase 3
	FBXDataContainerSystem* fbxSys = FBXDataContainerSystem::GetInstance();

	FBXCharacterData* terrainFbx = new FBXCharacterData();

	if (FAILED(fbxSys->LoadModelFBX(L"./Resources/fbx/TerrainSample.fbx", L"TerrainSample")))
		return E_FAIL;
	terrainFbx->SetMainFBX(L"TerrainSample");

	GameObject* terrainObj = new GameObject(terrainFbx);

	//地形用コンポーネント作成。
	TerrainComponent* trCom = new TerrainComponent();
	terrainObj->addComponent(trCom);

	//ImGui Set
	m_imguiManager->setImguiObject("GameScene_Stage", trCom,
		DEBUG_FLAG::Scene_InGameAll | DEBUG_FLAG::Component_Terrain | DEBUG_FLAG::Shader_Stage);

	AddSceneObject(terrainObj);

	//m_terrainsにTerrainComponentを登録
	m_terrains.push_back(trCom);

	//===動く地形 FBX読み込み
	if (FAILED(fbxSys->LoadModelFBX(L"./Resources/fbx/Platform.fbx", L"Platform")))
		return E_FAIL;

	//スカイドーム
	FBXCharacterData* skydomeFbx = new FBXCharacterData();
	if (FAILED(fbxSys->LoadModelFBX(L"./Resources/fbx/SkyDome001.fbx", L"SkyDome")))
		return E_FAIL;

	skydomeFbx->SetMainFBX(L"SkyDome");
	GameObject* skydomeObj = new GameObject(skydomeFbx);
	SkyDomeComponent* skCom = new SkyDomeComponent();
	skydomeObj->addComponent(skCom);

	AddSceneObject(skydomeObj);

	//Unityちゃん登録
	GameObject* unityChanObj;
	FBXCharacterData* unityChanFbx = new FBXCharacterData();	//FBX用CharacterData
	UnityChanPlayer* unityChanPlayer = new UnityChanPlayer();	//UnityChan本体

	//Phase 3 UnityちゃんFBX読み込み
	fbxSys->LoadModelFBX(L"./Resources/fbx/unitychan.fbx", L"UnityChan");
	fbxSys->LoadAnimationFBX(L"./Resources/fbx/UnityChanAnime/unitychan_WAIT00.fbx", L"WAIT00");
	fbxSys->LoadAnimationFBX(L"./Resources/fbx/UnityChanAnime/unitychan_WALK00_F.fbx", L"WALK_F");
	fbxSys->LoadAnimationFBX(L"./Resources/fbx/UnityChanAnime/unitychan_UMATOBI00.fbx", L"JUMP");

	unityChanObj = new GameObject(unityChanFbx);	//FBXCharacterDataを持たせて初期化
	unityChanObj->addComponent(unityChanPlayer);	//UnityChan本体コンポーネントをセット
	GameAccessHub::setUnityChan(unityChanPlayer);
	//ImGui Set
	m_imguiManager->setImguiObject("GameScene_UnityChanPlayer", unityChanPlayer,
		DEBUG_FLAG::Scene_InGameAll | DEBUG_FLAG::Component_Player | DEBUG_FLAG::Shader_Player);
	AddSceneObject(unityChanObj);

	//カメラ
	GameObject* cameraObj;
	cameraObj = new GameObject(new CharacterData());
	CameraComponent* camComp = new CameraComponent();

	//カメラリスト追加 UIモード時用
	//====CameraFix
	m_cameraComponents[L"MainCamera"] = camComp;

	cameraObj->addComponent(camComp);
	AddSceneObject(cameraObj);

	//本当はCameraComponent本体より先に動いて欲しい
	FlyingCameraController* flyCam = new FlyingCameraController();
	cameraObj->addComponent(flyCam);

	ThirdPersonCameraController* tpCam = new ThirdPersonCameraController();
	cameraObj->addComponent(tpCam);
	//ImGui Set
	m_imguiManager->setImguiObject("GameCam_ThirdPersonCamera", tpCam, DEBUG_FLAG::Scene_InGameAll);

	tpCam->setActive(false);	//初期状態OFFに

	CameraChangerComponent* camChanger = new CameraChangerComponent();
	cameraObj->addComponent(camChanger);	//CameraChangerをセット

	camChanger->SetCameraController(flyCam);//CameraChangerに高高度カメラセット
	camChanger->SetCameraController(tpCam);	//CameraChangerに３人称カメラセット

	camChanger->ChangeCameraController(0);	//初期状態を高高度カメラに設定
	//カメラController系ここまで

	//本当はカメラも複数個用意して、カメラごとに同時レンダリング出来るようにしたい
	engine->SetCameraData(cameraObj->getCharacterData());

	//カメラ初期設定
	camComp->changeCameraRatio(engine->GetWidth(), engine->GetHeight());
	camComp->changeCameraDepth(0.01f, 1000.0f);
	camComp->changeCameraFOVRadian(DirectX::XMConvertToRadians(45.0f));

	camComp->changeCameraPosition(1.5f, 1.2f, 0.0f);

	//UnityChanPlayerにCameraComponentを登録
	unityChanPlayer->SetCurrentCamera(camComp);

	//=====Camera Change Phase 1
	cameraObj = new GameObject(new CharacterData());
	GamePrograming3UIRender* uiRender = new GamePrograming3UIRender();
	cameraObj->addComponent(uiRender);

	AddSceneObject(cameraObj);

	//カメラリスト追加 UIモード時用
	m_cameraComponents[L"HUDCamera"] = uiRender;
	//=====Camera Change Phase 1 End

	//======HeartItem FBX読み込み処理
	if (FAILED(fbxSys->LoadModelFBX(L"./Resources/fbx/Heart.fbx", L"GoldenHeart")))
		return E_FAIL;

	if (FAILED(fbxSys->LoadModelFBX(L"./Resources/fbx/Heart.fbx", L"BlueHeart")))
		return E_FAIL;

	if (FAILED(fbxSys->LoadModelFBX(L"./Resources/fbx/Heart.fbx", L"RedHeart")))
		return E_FAIL;
	//======HeartItem FBX読み込み処理 End

	return hr;
}