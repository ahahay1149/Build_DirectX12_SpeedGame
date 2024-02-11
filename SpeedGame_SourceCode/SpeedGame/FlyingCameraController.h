#pragma once
#include <GameObject.h>
#include "CameraComponent.h"

class FlyingCameraController :
	public GameComponent
{
private:
	CameraComponent* m_camera;

	XMFLOAT3		m_targetPos;

public:
	void initAction() override;		//コンポーネント初期化時に呼ばれる処理
	bool frameAction() override;	//毎フレーム呼ばれる処理
	void finishAction() override;	//終了時に呼ばれる処理
};