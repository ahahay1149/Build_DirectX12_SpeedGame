#include "GamePrograming3Scene.h"
#include "CameraChangerComponent.h"

void CameraChangerComponent::initAction()
{
	GamePrograming3Scene* scene = dynamic_cast<GamePrograming3Scene*>(MyAccessHub::getMyGameEngine()->GetSceneController());
	m_keyBind = dynamic_cast<KeyBindComponent*>(scene->getKeyComponent());
	m_currentCamera = -1;
}

bool CameraChangerComponent::frameAction()
{
	//カメラチェンジテスト
	if (m_keyBind->getCurrentInputState(InputManager::BUTTON_STATE::BUTTON_DOWN, KeyBindComponent::BUTTON_IDS::MOUSE_R))
	{
		int index = (m_currentCamera + 1) % m_cameraComponents.size();

		ChangeCameraController(index);
	}

	return true;
}

void CameraChangerComponent::finishAction()
{
	m_cameraComponents.clear();
}

void CameraChangerComponent::SetCameraController(GameComponent* camCon)
{
	m_cameraComponents.push_back(camCon);
	camCon->setActive(false);
}

void CameraChangerComponent::ChangeCameraController(int index)
{
	if (index >= m_cameraComponents.size()) return;

	GameComponent* cam = m_cameraComponents[index];
	if (cam != nullptr)
	{
		//新しいCameraをONにして現在のCameraをOFF
		if (m_currentCamera > -1)
			m_cameraComponents[m_currentCamera]->setActive(false);

		m_currentCamera = index;
		cam->setActive(true);
	}
}