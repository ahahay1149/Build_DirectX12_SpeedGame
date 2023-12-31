#include <MyAccessHub.h>
#include "CameraComponent.h"
#include "FlyingCameraController.h"

void FlyingCameraController::initAction()
{
	//自身とセットになっているCameraComponentを探す
	//CameraComponentは絶対にあるという前提の構造。なかったらframeActionで落ちる
	auto components = getGameObject()->getComponents();

	m_camera = nullptr;
	for (auto comp : components)
	{
		m_camera = dynamic_cast<CameraComponent*>(comp);
		if (m_camera != nullptr)
		{
			break;
		}
	}

}

bool FlyingCameraController::frameAction()
{
	//01: Flying Camera
	//Focusに注目点（プレイヤの頭座標）が入っている
	m_targetPos = m_camera->getCameraFocus();

	//頭上上空にカメラセット
	m_camera->changeCameraPosition(m_targetPos.x, m_targetPos.y + 3.0f, m_targetPos.z - 2.0f);
	m_camera->changeCameraFocus(m_targetPos.x, m_targetPos.y, m_targetPos.z);
	//01: ここまで

	return true;
}

void FlyingCameraController::finishAction()
{
}