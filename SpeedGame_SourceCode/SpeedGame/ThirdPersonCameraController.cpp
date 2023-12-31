#include <MyAccessHub.h>
#include "CameraComponent.h"
#include "ThirdPersonCameraController.h"

#include "GamePrograming3Scene.h"

//ImGui
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"


void ThirdPersonCameraController::initAction()
{
	GamePrograming3Scene* scene = dynamic_cast<GamePrograming3Scene*>(MyAccessHub::getMyGameEngine()->GetSceneController());
	m_keyBind = dynamic_cast<KeyBindComponent*>(scene->getKeyComponent());

	auto components = getGameObject()->getComponents();

	m_camera = nullptr;
	for (auto comp : components)
	{
		m_camera = dynamic_cast<CameraComponent*>(comp);
		if (m_camera != nullptr)	//dynamic_castは失敗するとnullptr
		{
			break;
		}
	}

	m_hDeg = 0.0f;
	m_vDeg = 0.0f;
	m_distance = 1.5f;
}

//三人称カメラ
bool ThirdPersonCameraController::frameAction()
{
	int move_x, move_y;
	CharacterData* chData = getGameObject()->getCharacterData();
	float height = m_camera->getSetHeight();
	float width = m_camera->getSetWidth();

	if (m_keyBind->getCurrentInputType() == KeyBindComponent::INPUT_TYPES::KEYBOARD)
	{
		//01:マウス座標から角度を生成
		XMINT2 mousePos = MyAccessHub::getMyGameEngine()->GetInputManager()->getMousePosition();

		float half_h = height * 0.5f;	//高さの半分
		//float quat_w = width * 0.25f;	//幅は1/4
		float quat_w = width * 0.125f;	//幅は1/8

		move_x = mousePos.x - quat_w;	//左右はWindow幅全体で１回転出来る
		move_y = mousePos.y - half_h;	//上下は180度で止まるようにしている

		m_vDeg = move_y / half_h * 90.0f;	//MAX+-90度（特に上下は90度を越えないように。越えると急に見ている向きが変わる）
		m_hDeg = move_x / quat_w * 90.0f;
		//01:ここまで
	}
	else
	{
		//パッドはアナログ値の加算
		float f_temp = 0.0f;

		move_x = m_keyBind->getCurrentAnalogValue(KeyBindComponent::ANALOG_IDS::CAMERA_H);
		move_y = m_keyBind->getCurrentAnalogValue(KeyBindComponent::ANALOG_IDS::CAMERA_V);

		f_temp = move_y / (height * 0.5f) * 90.0f;	//MAX90度
		m_vDeg += f_temp;
		m_vDeg = m_vDeg < -90.0f ? -90.0f : m_vDeg > 90.0f ? 89.9999f : m_vDeg;	//ジャスト90.0fになると計算がバグって回転が0になるので89.9999で設定

		f_temp = move_x / (width * 0.25f) * 90.0f;	//横は180度回ってもそれほど問題にはならない
		m_hDeg += f_temp;
		m_hDeg = m_hDeg < -180.0f ? m_hDeg + 360.0f : m_hDeg > 180.0f ? m_hDeg - 360.0f : m_hDeg;
	}

	//02: マウス入力からカメラの位置をフォーカス点を中心に移動
	//角度ゼロ、原点中心のカメラ座標を作ってX軸とY軸で回転
	XMVECTOR pos = { 0.0f, 0.0f, m_distance, 0.0f };
	XMMATRIX mat = XMMatrixRotationX(XMConvertToRadians(m_vDeg))* XMMatrixRotationY(XMConvertToRadians(m_hDeg));

	XMFLOAT3 focus = m_camera->getCameraFocus();
	mat = mat * XMMatrixTranslation(focus.x, focus.y, focus.z);	//回転マトリクスに平行移動を掛ける（中心の移動）

	pos = XMVector3Transform(pos, mat);	//座標に完成したマトリクスを掛けてアフィン変換を座標に反映する

	//出力を座標に反映する
	m_camera->changeCameraPosition(pos.m128_f32[0], pos.m128_f32[1], pos.m128_f32[2]);
	//02: ここまで

	imgui();

	return true;
}

void ThirdPersonCameraController::finishAction()
{
}

void ThirdPersonCameraController::imgui()
{
	ImGui::Begin("Window");
	ImGui::Checkbox("ThirdPersonCameraController", &check);
	ImGui::End();
	
	if (check == true)
	{
		ImGui::Begin("ThirdPersonCameraController");
		ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_::ImGuiCond_FirstUseEver);
	
		ImGui::SliderFloat("Distance", &m_distance, 0.3f, 2.0f);
		ImGui::SliderFloat("vDeg", &m_vDeg, -1000.0f, 1000.0f);
		ImGui::SliderFloat("hDeg", &m_hDeg, -1000.0f, 1000.0f);
	
		ImGui::End();
	}
}