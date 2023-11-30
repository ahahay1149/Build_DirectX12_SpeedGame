﻿#include "GamePrograming3Scene.h"

GamePrograming3Scene* GamePrograming3Scene::m_Scene = nullptr;
UnityChanPlayer* GamePrograming3Scene::m_UnityChan = nullptr;
GamePrograming3UIRender* GamePrograming3Scene::m_UIRender = nullptr;
ResultUIRender* GamePrograming3Scene::m_ResultUIRender = nullptr;

#include "GamePrograming3Enum.h"
#include "MyAccessHub.h"
#include "KeyBindComponent.h"

#include "SpriteRenderPipeline.h"
#include "FBXCharacterData.h"
#include "StandardFbxPipeline.h"

//Fbx Loadの最適化（一つのメッシュを使い回せるように処理を変更）
#include "FBXDataContainerSystem.h"

//GameObjects
#include "SkyDomeComponent.h"
#include "TerrainComponent.h"
#include "CameraComponent.h"

#include "UnityChanPlayer.h"

#include "CameraChangerComponent.h"			//カメラ切り替え
#include "FlyingCameraController.h"			//高高度カメラ
#include "ThirdPersonCameraController.h"	//三人称カメラ

#include "TitleUIRender.h"					//タイトルUI
#include "GameOverUIRender.h"				//ゲームオーバーUI

//=====Camera Change Phase 1
#include "GamePrograming3UIRender.h"
//=====Camera Change Phase 1 End

//HitTest
#include "HeartItemComponent.h"

//=====動く地形
#include "MovingPlatform.h"
//=====動く地形 END

//=====Change Scene
#include "TitleSceneSample.h"
//=====Change Scene END


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
	setScene(this);

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

		//=====Change Scene
		ClearSceneObjects();
		//=====Change Scene End

		//Phase 3 Fbx管理を別の処理にまとめたため、Fbx読み込み指示がここに集中
		switch (m_scene)
		{
			case static_cast<UINT>(GAME_SCENES::IN_GAME):
			{
				FBXDataContainerSystem* fbxSys = FBXDataContainerSystem::GetInstance();
				fbxSys->DeleteModelFBX(L"SkyDome");
				fbxSys->DeleteModelFBX(L"TerrainSample");
				fbxSys->DeleteModelFBX(L"UnityChan");
				fbxSys->DeleteModelFBX(L"Platform");
				fbxSys->DeleteModelFBX(L"GoldenHeart");

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
				engine->GetTextureManager()->CreateTextureFromFile(engine->GetDirect3DDevice(), L"Sprite00", L"./Resources/textures/texture.png");
				//=====Camera Change Phase 1
				engine->GetTextureManager()->CreateTextureFromFile(engine->GetDirect3DDevice(), L"HUDTexture", L"./Resources/textures/HUD/UITexture.png");
				//=====Camera Change Phase 1 End
				//=====Change Scene
				engine->GetTextureManager()->CreateTextureFromFile(engine->GetDirect3DDevice(), L"TitleTexture", L"./Resources/textures/Title/TitleSample.png");
				//=====Change Scene End
				engine->GetMeshManager()->createPresetMeshData();

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
				StandardFbxPipeline* fbxPL = new StandardFbxPipeline();
				//fbxPL->SetAnimationMode(false);
				plMng->AddPipeLineObject(L"StaticFBX", fbxPL);

				fbxPL = new StandardFbxPipeline();
				fbxPL->SetAnimationMode(true);
				plMng->AddPipeLineObject(L"AnimationFBX", fbxPL);

				//==========PipeLineManager END

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
				if (!soMng->loadSoundFile(L"./Resources/sounds/GameClearJingle.wav", soundId))		//01 GameClearJingle
					return E_FAIL;
				if (!soMng->loadSoundFile(L"./Resources/sounds/GameOverJingle.wav", soundId))		//02 GameOverJingle
					return E_FAIL;
				if (!soMng->loadSoundFile(L"./Resources/sounds/getHeartBell0.wav", soundId))		//03
					return E_FAIL;
				if (!soMng->loadSoundFile(L"./Resources/sounds/getHeartBell1.wav", soundId))		//04
					return E_FAIL;
				if (!soMng->loadSoundFile(L"./Resources/sounds/getHeartBell2.wav", soundId))		//05
					return E_FAIL;
				if (!soMng->loadSoundFile(L"./Resources/sounds/getHeartBell3.wav", soundId))		//06
					return E_FAIL;
				if (!soMng->loadSoundFile(L"./Resources/sounds/getHeartBell4.wav", soundId))		//07
					return E_FAIL;
				if (!soMng->loadSoundFile(L"./Resources/sounds/getHeartBell5.wav", soundId))		//08
					return E_FAIL;


				//システム制御統合オブジェクト登録
				m_systemObject = make_unique<GameObject>(nullptr);
				m_keyComponent = new KeyBindComponent();
				m_systemObject->addComponent(m_keyComponent);
				engine->AddGameObject(m_systemObject.get());

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

				titleCamera->SetNextScene(GAME_SCENES::IN_GAME);
				titleCamera->SetBGColor(0.5f, 0.5f, 0.5f);
				titleCamera->SetFontPosition(74.0f);

				AddSceneObject(cameraObj);

				//カメラリスト追加 UIモード時用
				m_cameraComponents[L"TitleCamera"] = titleCamera;
			}
			break;

		case static_cast<UINT>(GAME_SCENES::IN_GAME):
			{
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

				AddSceneObject(terrainObj);
				
				//m_terrainsにTerrainComponentを登録
				m_terrains.push_back(trCom);

				//=========動く地形　対応
				//Platform Test
				terrainFbx = new FBXCharacterData();
				terrainFbx->setPosition(10.0f, 0.5f, 10.0f);
				if (FAILED(fbxSys->LoadModelFBX(L"./Resources/fbx/Platform.fbx", L"Platform")))
					return E_FAIL;
				terrainFbx->SetMainFBX(L"Platform");

				terrainObj = new GameObject(terrainFbx);

				//地形用コンポーネント作成
				trCom = new TerrainComponent();
				terrainObj->addComponent(trCom);

				terrainObj->addComponent(new MovingPlatform());

				AddSceneObject(terrainObj);

				//m_terrainsにTerrainComponentを登録
				m_terrains.push_back(trCom);
				//=========動く地形　対応 END

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
				setUnityChan(unityChanPlayer);					//UnityChan本体をAccessHubにセット
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
				setUIRender(uiRender);

				AddSceneObject(cameraObj);

				//カメラリスト追加 UIモード時用
				m_cameraComponents[L"HUDCamera"] = uiRender;
				//=====Camera Change Phase 1 End

				FBXCharacterData* heartFbx = new FBXCharacterData();

				if (FAILED(fbxSys->LoadModelFBX(L"./Resources/fbx/Heart.fbx", L"GoldenHeart")))
					return E_FAIL;

				GameObject* heartObj;	//使いまわしするのでここで宣言

				//====01====
				heartFbx = new FBXCharacterData();
				heartFbx->SetMainFBX(L"GoldenHeart");
				heartFbx->setPosition(3.0f, 0.0f, 8.0f);

				heartObj = new GameObject(heartFbx);
				HeartItemComponent* heartItem01 = new HeartItemComponent();
				heartItem01->setPlayerSpeed(0.02f);
				heartObj->addComponent(heartItem01);
				AddSceneObject(heartObj);

				//====02====
				heartFbx = new FBXCharacterData();
				heartFbx->SetMainFBX(L"GoldenHeart");
				heartFbx->setPosition(-11.0f, -4.0f, 10.0f);

				heartObj = new GameObject(heartFbx);
				HeartItemComponent* heartItem02 = new HeartItemComponent();
				heartItem02->setPlayerSpeed(0.04f);
				heartObj->addComponent(heartItem02);
				AddSceneObject(heartObj);

				//====03====
				heartFbx = new FBXCharacterData();
				heartFbx->SetMainFBX(L"GoldenHeart");
				heartFbx->setPosition(13.0f, -3.0f, 22.0f);

				heartObj = new GameObject(heartFbx);
				HeartItemComponent* heartItem03 = new HeartItemComponent();
				heartItem03->setPlayerSpeed(0.02f);
				heartObj->addComponent(heartItem03);
				AddSceneObject(heartObj);

				//====04====
				heartFbx = new FBXCharacterData();
				heartFbx->SetMainFBX(L"GoldenHeart");
				heartFbx->setPosition(0.0f, -6.0f, 20.0f);

				heartObj = new GameObject(heartFbx);
				HeartItemComponent* heartItem04 = new HeartItemComponent();
				heartItem04->setPlayerSpeed(0.02f);
				heartObj->addComponent(heartItem04);
				AddSceneObject(heartObj);

				//====05====
				heartFbx = new FBXCharacterData();
				heartFbx->SetMainFBX(L"GoldenHeart");
				heartFbx->setPosition(-7.0f, -4.0f, 18.0f);

				heartObj = new GameObject(heartFbx);
				HeartItemComponent* heartItem05 = new HeartItemComponent();
				heartItem05->setPlayerSpeed(0.02f);
				heartObj->addComponent(heartItem05);
				AddSceneObject(heartObj);

				//====06====
				heartFbx = new FBXCharacterData();
				heartFbx->SetMainFBX(L"GoldenHeart");
				heartFbx->setPosition(20.0f, 0.0f, 20.0f);

				heartObj = new GameObject(heartFbx);
				HeartItemComponent* heartItem06 = new HeartItemComponent();
				heartItem06->setPlayerSpeed(0.02f);
				heartObj->addComponent(heartItem06);
				AddSceneObject(heartObj);

				//====07====
				heartFbx = new FBXCharacterData();
				heartFbx->SetMainFBX(L"GoldenHeart");
				heartFbx->setPosition(25.0f, 1.0f, 15.0f);

				heartObj = new GameObject(heartFbx);
				HeartItemComponent* heartItem07 = new HeartItemComponent();
				heartItem07->setPlayerSpeed(0.03f);
				heartObj->addComponent(heartItem07);
				AddSceneObject(heartObj);

				//====08====
				heartFbx = new FBXCharacterData();
				heartFbx->SetMainFBX(L"GoldenHeart");
				heartFbx->setPosition(7.0f, -8.0f, 30.0f);

				heartObj = new GameObject(heartFbx);
				HeartItemComponent* heartItem08 = new HeartItemComponent();
				heartItem08->setPlayerSpeed(0.01f);
				heartObj->addComponent(heartItem08);
				AddSceneObject(heartObj);

				//====09====
				heartFbx = new FBXCharacterData();
				heartFbx->SetMainFBX(L"GoldenHeart");
				heartFbx->setPosition(15.0f, 0.0f, 10.0f);

				heartObj = new GameObject(heartFbx);
				HeartItemComponent* heartItem09 = new HeartItemComponent();
				heartItem09->setPlayerSpeed(0.02f);
				heartObj->addComponent(heartItem09);
				AddSceneObject(heartObj);

				//====10====
				heartFbx = new FBXCharacterData();
				heartFbx->SetMainFBX(L"GoldenHeart");
				heartFbx->setPosition(10.0f, 15.0f, 10.0f);

				heartObj = new GameObject(heartFbx);
				HeartItemComponent* heartItem10 = new HeartItemComponent();
				heartItem10->setPlayerSpeed(0.1f);
				heartObj->addComponent(heartItem10);
				AddSceneObject(heartObj);

				//====HeartSet End====

				//InGame BGM
				SoundManager* soMng = engine->GetSoundManager();
				soMng->stop(playingMusic[1]);
				soMng->stop(playingMusic[2]);
				playingMusic[0] = soMng->play(0);

				engine->UploadCreatedTextures();

			}
			break;

		case static_cast<UINT>(GAME_SCENES::GAME_OVER):
			{
				SoundManager* soMng = engine->GetSoundManager();
				soMng->stop(playingMusic[0]);
				playingMusic[1] = soMng->play(2);

				GameObject* CameraObj = new GameObject(new CharacterData());
				TitleSceneSample* overCamera = new TitleSceneSample();
				CameraObj->addComponent(overCamera);

				GameOverUIRender* uiRender = new GameOverUIRender();
				CameraObj->addComponent(uiRender);

				overCamera->SetNextScene(GAME_SCENES::TITLE);
				overCamera->SetBGColor(0.5f, 0.5f, 0.5f);
				overCamera->SetFontPosition(200.0f);

				AddSceneObject(CameraObj);

				//カメラリスト追加 UIモード時用
				m_cameraComponents[L"TitleCamera"] = overCamera;
			}
			break;

		case static_cast<UINT>(GAME_SCENES::GAME_CLEAR):
			{
				SoundManager* soMng = engine->GetSoundManager();
				soMng->stop(playingMusic[0]);
				playingMusic[2] = soMng->play(1);

				GameObject* cameraObj = new GameObject(new CharacterData());
				TitleSceneSample* clearCamera = new TitleSceneSample();
				cameraObj->addComponent(clearCamera);

				//======Camera Change Phase 1
				ResultUIRender* uiRender = new ResultUIRender();
				cameraObj->addComponent(uiRender);
				setResultUIRender(uiRender);

				//カメラリスト追加 UIモード時用
				m_cameraComponents[L"HUDCamera"] = uiRender;
				//======Camera Change Phase 1 End

				clearCamera->SetNextScene(GAME_SCENES::TITLE);
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
