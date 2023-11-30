#pragma once

#include <GameObject.h>
#include "KeyBindComponent.h"
#include "CameraComponent.h"

class ThirdPersonCameraController : public GameComponent
{
private:
	KeyBindComponent* m_keyBind;
	CameraComponent* m_camera;

	float			m_hDeg;
	float			m_vDeg;
	float			m_distance;

	void initAction() override;			//コンポーネント初期化時に呼ばれる処理

public:
	bool frameAction() override;		//毎フレーム呼ばれる処理　falseを返すとこのコンポーネントは終了し削除される
	void finishAction() override;		//終了時に呼ばれる処理
};