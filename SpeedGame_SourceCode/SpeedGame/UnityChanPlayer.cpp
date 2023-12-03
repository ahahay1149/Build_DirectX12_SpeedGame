#include "UnityChanPlayer.h"
#include "FBXCharacterData.h"	//FBXCharacterDataを使うので

#include "GamePrograming3Scene.h"
#include "KeyBindComponent.h"
#include "GamePrograming3Enum.h"

//Phase3
#include "FBXDataContainerSystem.h"

#include "GamePrograming3UIRender.h"

#define MOVE_3D (0)				//(0)にすると2Dモード

void UnityChanPlayer::initAction()
{
	//FBXCharacterDataは基底クラスのGameObjectにセットする
	FBXCharacterData* chdata = static_cast<FBXCharacterData*>(getGameObject()->getCharacterData());
	//chdata->SetGraphicsPipeLine(L"StaticFBX");	//通常FBX
	//chdata->SetGraphicsPipeLine(L"AnimationFBX");	//スキンアニメ有りFBX
	chdata->SetGraphicsPipeLine(L"SkeltalLambert");	//スキンアニメ有りFBX+Lambert

	chdata->SetMainFBX(L"UnityChan");

	chdata->setScale(0.01f, 0.01f, 0.01f);	//元モデルがかなり大きい（というかスカイドームとかとあってない）ので縮小
	chdata->setPosition(0.0f, 0.0f, 0.0f);	//初期値設定

	//Unityちゃんの判定設定を作る。
	XMFLOAT3 min = chdata->GetMainFbx()->GetFbxMin();	//モデル全体の一番値の小さい座標	
	XMFLOAT3 max = chdata->GetMainFbx()->GetFbxMax();	//モデル全体の一番値の大きい座標

	float headY = (max.y - min.y) * 0.8f;								//Unityちゃんの頭座標をなんとなくで出す
	m_unityChanHeadHeight = headY * chdata->getScale().y;				//スケールかけて空間中の高さに
	//足元の「抜ける」高さを設定。坂の上り下り。
	m_walkableHeight = (max.y - min.y) * 0.25f * chdata->getScale().y;

	chdata->SetAnime(L"WAIT00");

	//Unityちゃん本体のヒット判定作成　円柱型
	XMFLOAT3 scl = chdata->getScale();
	m_hitHeight = (max.y - min.y) * scl.y;	//scale
	bodyColl.setHeight(m_hitHeight * 0.9f);
	bodyColl.setRadius(m_hitHeight * 0.55f * 0.5f);	//直径がメッシュ幅の55%(Tボーンなので) / 半径なのでさらに半分
	bodyColl.setAttackType(static_cast<UINT>(HIT_ORDER::HIT_PLAYER_BODY), 10);

	//=========UnityChan　飛ぶ
	m_gravityPower = 9.8f * 0.001f;	//疑似重力の値
	m_terminalVelocity = 2.0f;	//終端速度
	m_YSpeed = 0.0f;
	m_jumpPower = 0.2f;			//ジャンプ力
	//=========UnityChan　飛ぶ End
	
	//=====動く地形 対応
	m_onGround = false;			//初期状態地面なし。フラグも変更
	m_currentTerrain = nullptr;	//初期状態地面なし
	m_lastMatrix = XMMatrixIdentity();
	//=====動く地形 対応 END

}

bool UnityChanPlayer::frameAction()
{
	FBXCharacterData* chData = static_cast<FBXCharacterData*>(getGameObject()->getCharacterData());

	//移動処理
	GamePrograming3Scene* scene = static_cast<GamePrograming3Scene*>(MyAccessHub::getMyGameEngine()->GetSceneController());
	KeyBindComponent* keyBind = static_cast<KeyBindComponent*>(scene->getKeyComponent());
	XMFLOAT3 moveVect = {};	//ボタン入力を方向に見立ててベクトルを作成

	//地形判定
	//=====動く地形対応　Terrainが複数ある事を前提にする
	TerrainComponent* terCom = nullptr;
	//=====動く地形 対応 END
	XMFLOAT3 hitPos = {};       //判定接触点（結果用）
	XMFLOAT3 hitNormal = {};    //接触ポリゴンの法線（結果用）
	XMFLOAT3 rayStart;          //線分開始点
	XMFLOAT3 rayEnd;            //線分終了点
	HitRayLine ray;             //線分判定（Ray）

	//=====動く地形 対応
	if (m_currentTerrain != nullptr)
	{
		//地形移動
		CharacterData* trData = m_currentTerrain->getGameObject()->getCharacterData();
		XMMATRIX& nowMat = trData->GetWorldMatrix();

		XMFLOAT3 pos = chData->getPosition();
		XMVECTOR posVect = XMLoadFloat3(&pos);

		float rad = XMConvertToRadians(chData->getRotation().y);
		//Xがひっくり返るのを忘れちゃいけない
		XMFLOAT3 rot{pos.x - cos(rad), pos.y, pos.z + sin(rad)};

		XMVECTOR rotVect = XMLoadFloat3(&rot);

		posVect = XMVector3Transform(posVect, m_lastMatrix);	//前の逆行列をかける
		rotVect = XMVector3Transform(rotVect, m_lastMatrix);

		posVect = XMVector3Transform(posVect, nowMat);	//今の行列をかける
		rotVect = XMVector3Transform(rotVect, nowMat);

		m_lastMatrix = trData->GetInverseWorldMatrix();	//今の逆行列を保存

		chData->setPosition(XMVectorGetX(posVect), XMVectorGetY(posVect), XMVectorGetZ(posVect));
		pos = chData->getPosition();
		//Xがひっくり返るのを忘れずに
		chData->setRotation(0.0f, XMConvertToDegrees( atan2(XMVectorGetZ(rotVect) - pos.z, -(XMVectorGetX(rotVect) - pos.x)) ), 0.0f);
	}
	else
	{
		m_onGround = false;
		m_lastMatrix = XMMatrixIdentity();
	}
	//=====動く地形 対応 END

	//入力モードで処理を分岐
	switch (keyBind->getCurrentInputType())
	{
		case KeyBindComponent::INPUT_TYPES::KEYBOARD:
		{
			//X方向
			if (keyBind->getCurrentInputState(InputManager::BUTTON_STATE::BUTTON_PRESS, KeyBindComponent::BUTTON_IDS::MOVE_LEFT))
			{
				moveVect.x = -1.0f;	//左
			}
			else if (keyBind->getCurrentInputState(InputManager::BUTTON_STATE::BUTTON_PRESS, KeyBindComponent::BUTTON_IDS::MOVE_RIGHT))
			{
				moveVect.x = 1.0f;	//右
			}

			//Z方向
			if (keyBind->getCurrentInputState(InputManager::BUTTON_STATE::BUTTON_PRESS, KeyBindComponent::BUTTON_IDS::MOVE_BACK))
			{
				moveVect.z = -1.0f;	//下（手前）
			}
			else if (keyBind->getCurrentInputState(InputManager::BUTTON_STATE::BUTTON_PRESS, KeyBindComponent::BUTTON_IDS::MOVE_FORWARD))
			{
				moveVect.z = 1.0f;	//上（奥）
			}

			//斜め移動対応
			if (moveVect.x != 0.0f && moveVect.z != 0.0f)
			{
				//ルート２で割って正規化
				moveVect.x /= 1.41421356f;
				moveVect.z /= 1.41421356f;
			}


			//=========UnityChan　飛ぶ
			if (m_onGround)
			{
				if (keyBind->getCurrentInputState(InputManager::BUTTON_STATE::BUTTON_DOWN, KeyBindComponent::BUTTON_IDS::BTN_JUMP))
				{
					m_onGround = false;
					m_YSpeed = m_jumpPower;

					chData->SetAnime(L"JUMP");
				}
			}
			//=========UnityChan　飛ぶ End

		}
			break;

		case KeyBindComponent::INPUT_TYPES::XINPUT_0:
		{
			moveVect.x = keyBind->getAnalogValue(KeyBindComponent::INPUT_TYPES::XINPUT_0, KeyBindComponent::ANALOG_IDS::MOVE_H);
			moveVect.z = keyBind->getAnalogValue(KeyBindComponent::INPUT_TYPES::XINPUT_0, KeyBindComponent::ANALOG_IDS::MOVE_V);
		}
			break;
	}

	//カメラ角度の反映
	//カメラの方向ベクトル取得
	XMFLOAT3 camVect = m_currentCamera->getCameraDirection();
	XMVECTOR rotMove = {};		//回転計算用バッファ
	float xzRadian;	//Yaw
	xzRadian = atan2f(camVect.x, camVect.z);			//Y軸回転角
	XMMATRIX qtXZ = XMMatrixRotationY(xzRadian);		//Y軸回転行列 Yaw

#if MOVE_3D
	// 3D版
	// 移動なのでRollは要らない

	//逆回転を忘れずに
	XMVECTOR workVect = XMVector3Transform(XMLoadFloat3(&camVect), XMMatrixRotationY(-xzRadian));

	float yzRadian;	//Pitch
	yzRadian = atan2f(-XMVectorGetY(workVect), XMVectorGetZ(workVect));
	XMMATRIX qtYZ = XMMatrixRotationX(yzRadian); //X軸回転行列 Pitch
	
	// moveVectに変形行列を掛ける
	rotMove = XMVector3Transform(XMLoadFloat3(&moveVect), qtYZ * qtXZ);

	//Unityちゃんを移動方向に向ける
	chData->setRotation(XMConvertToDegrees(yzRadian), XMConvertToDegrees(xzRadian), 0.0f);
#else
	// 2D版
	rotMove = XMVector3Transform(XMLoadFloat3(&moveVect), qtXZ);

	// 入力処理があった時のみ回転を更新
	if (moveVect.x != 0.0f || moveVect.z != 0.0f)
	{
		chData->setRotation(0.0f, XMConvertToDegrees(atan2f(XMVectorGetX(rotMove), XMVectorGetZ(rotMove))), 0.0f);

		//=========UnityChan　飛ぶ
		if (m_onGround)
			chData->SetAnime(L"WALK_F");	//歩行アニメ
	}
	else
	{
		//=========UnityChan　飛ぶ
		if (m_onGround)
			chData->SetAnime(L"WAIT00");	//停止アニメ
	}
#endif // MOVE_3D

	//結果のrotMoveをmoveVectに反映
	moveVect.x = XMVectorGetX(rotMove);
	moveVect.y = XMVectorGetY(rotMove);
	moveVect.z = XMVectorGetZ(rotMove);

	//出来た移動ベクトルに移動速度(0.05)をかける
	moveVect.x *= 0.05f + m_plusSpeed;
	moveVect.y *= 0.05f + m_plusSpeed / 2;
	moveVect.z *= 0.05f + m_plusSpeed;

	XMFLOAT3 nowPos = chData->getPosition();
	nowPos.x += moveVect.x;
	nowPos.y += moveVect.y;
	nowPos.z += moveVect.z;

	//=========UnityChan　飛ぶ
	nowPos.y += m_YSpeed;

	//==========地形判定追加
	bool canWalk = true;

	rayStart = chData->getPosition();
	rayStart.y += m_walkableHeight;
	rayEnd = nowPos;
	rayEnd.y += m_walkableHeight;

	ray.setLine(rayStart, rayEnd, 0.0f);

	//=====動く地形 対応

	for (int i = 0; (terCom = scene->getTerrainComponent(i)) != nullptr; i++)
	{
		if (terCom->RayCastHit(ray, hitPos, hitNormal))
		{
			//X,Zをヒット位置に補正
			nowPos.x = hitPos.x;
			nowPos.z = hitPos.z;

			break;
		}
	}

	//=====動く地形 対応 End

	rayStart = nowPos;
	rayEnd = nowPos;

	if (m_YSpeed < 0.0f)
	{
		rayStart.y += m_walkableHeight - m_YSpeed;
	}
	else
	{
		rayStart.y += m_walkableHeight;
	}
	rayEnd.y -= m_walkableHeight;

	ray.setLine(rayStart, rayEnd, 0.0f);

	//=====動く地形 対応
	TerrainComponent* backupTr = m_currentTerrain;	//地形比較用
	m_currentTerrain = nullptr;

	//canWalk = false;
	for (int i = 0; (terCom = scene->getTerrainComponent(i)) != nullptr; i++)
	{
 		if (terCom->RayCastHit(ray, hitPos, hitNormal))
		{
			//床の高さに現在位置を調整

			//=========UnityChan　飛ぶ
			if (m_onGround || m_YSpeed < 0.0f)
			{
				nowPos.y = hitPos.y;

				m_currentTerrain = terCom; //追従地形を設定
				if (backupTr != m_currentTerrain)
				{
					//地形の逆行列を取得（次回の移動対応）
					m_lastMatrix = terCom->getGameObject()->getCharacterData()->GetInverseWorldMatrix();
				}

				break;
			}
		}
	}

	if (m_currentTerrain != nullptr)
	{
		if (!m_onGround)
		{
			m_onGround = true;
			m_YSpeed = 0.0f;

			//着地処理が必要ならここから分岐
			chData->SetAnime(L"WAIT00");
		}
	}
	else
	{
		canWalk = false;	//床がない

		//=========UnityChan　飛ぶ
		m_onGround = false;
	}

	//=====動く地形 対応 END


	//==========地形判定ここまで

	//=========UnityChan　飛ぶ
	if (!m_onGround)
	{
		m_YSpeed -= m_gravityPower;
		if (m_YSpeed < -m_terminalVelocity)
		{
			m_YSpeed = -m_terminalVelocity;
		}
	}

	//最初のカウント時に移動制御の反映が行われないようにする
	//timerはヘッダ側で何秒待つか指定してる
	timer -= 0.0166666f;
	if (timer < 0)
	{
		//現在位置更新
		chData->setPosition(nowPos.x, nowPos.y, nowPos.z);

		//アニメを進める
		chData->UpdateAnimation();
	}

	//AddRenderObject自体のパラメータの読み込みは描画時まで行わないので何時やっても良いが、
	//frameActionの結果描画しない可能性もあるので最後あたりで処理
	chData->GetPipeline()->AddRenerObject(chData);

	XMFLOAT3 plPos = chData->getPosition(); //プレイヤ位置取得

	//カメラ制御 Focusを現在位置の「頭部」位置にする
	m_currentCamera->changeCameraFocus(plPos.x, plPos.y + m_unityChanHeadHeight, plPos.z);

	//Unityちゃん判定移動
	bodyColl.setCenter(plPos.x, plPos.y + m_hitHeight * 0.5f, plPos.z);
	//判定セット
	MyAccessHub::getMyGameEngine()->GetHitManager()->setHitArea(this, &bodyColl);

	return true;
}

void UnityChanPlayer::finishAction()
{
}

void UnityChanPlayer::hitReaction(GameObject* targetGo, HitAreaBase* hit)
{
	//HeartItemにぶつかったとき音を鳴らす
	MyAccessHub::getMyGameEngine()->GetSoundManager()->play(4 + m_getHeartItems);	//getHeartItem01.wavから

	//プレイヤークラス側でもハートの取得数をカウント
	m_getHeartItems++;

	//UIクラスに取得数を増やしたことを伝える
	GamePrograming3Scene::getUIRender()->plusHeartItemCount();
}
