#include "TitleSceneSample.h"

#include <MyAccessHub.h>
#include <D3D12Helper.h>

#include "GamePrograming3Scene.h"
#include "KeyBindComponent.h"		//使いたいのはこれ

TitleSceneSample::TitleSceneSample()
{
	m_bgSp = std::make_unique<SpriteCharacter>();
	m_messageSp = std::make_unique<SpriteCharacter>();

	m_bgSp->SetCameraLabel(L"TitleCamera");
	m_messageSp->SetCameraLabel(L"TitleCamera");

	m_bgSp->SetGraphicsPipeLine(L"Sprite");
	m_messageSp->SetGraphicsPipeLine(L"AlphaSprite");

	m_bgSp->setTextureId(L"TitleTexture");
	m_messageSp->setTextureId(L"TitleTexture");

	//BGは左上にある2*2ドット部分の白色を色変化をつけて使うだけ
	Texture2DContainer* tex = MyAccessHub::getMyGameEngine()->GetTextureManager()->GetTexture(m_bgSp->getTextureId());
	float invH = 1.0f / tex->fHeight;
	XMFLOAT4 r = { 0.0f, 0.0f, 1.0f * invH, 1.0f * invH};

	float h = MyAccessHub::getMyGameEngine()->GetHeight();
	float w = MyAccessHub::getMyGameEngine()->GetWidth();

	m_bgSp->SetSpritePattern(0, w, h, r);
	m_bgSp->setSpriteIndex(0);


	m_messageSp->setColorMix(SpriteCharacter::COLOR_MIX_OP::MIX_MUL);
	m_messageSp->setColor(1, 1, 1, 1);
}

void TitleSceneSample::SetBGColor(float r, float g, float b)
{
	m_bgSp->setColorMix(SpriteCharacter::COLOR_MIX_OP::MIX_MUL);
	m_bgSp->setColor(r, g, b, 1.0f);
}

void TitleSceneSample::SetFontPosition(float pxY)
{
	Texture2DContainer* tex = MyAccessHub::getMyGameEngine()->GetTextureManager()->GetTexture(m_messageSp->getTextureId());

	float invH = 1.0f / tex->fHeight;

	XMFLOAT4 r = { 0.0f, pxY * invH, 1.0f, 143.0f * invH };

	m_messageSp->SetSpritePattern(0, tex->fWidth, 143.0f, r);
	m_messageSp->setSpriteIndex(0);
}

void TitleSceneSample::SetNextScene(GAME_SCENES nextSc)
{
	m_nextScene = nextSc;
}

void TitleSceneSample::initAction()
{
	MyGameEngine* engine = MyAccessHub::getMyGameEngine();
	CharacterData* chData = getGameObject()->getCharacterData();

	engine->InitCameraConstantBuffer(chData);

	chData->setPosition(0.0f, 0.0f, 0.0f);

	//半分直接D3D触ってるようなもんだから良くはないんだけど、マトリクスを固定で作ってしまう
	XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);		//視点（カメラ）座標
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);		//フォーカスする（カメラが向く）座標
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);		//カメラの上方向単位ベクトル（カメラのロール軸）
	XMMATRIX view = XMMatrixTranspose(MakeViewMatix(Eye, At, Up));
	XMMATRIX proj = XMMatrixTranspose(MakeOrthographicPrjectionMatrix(engine->GetWidth(), engine->GetHeight(), 0.01f, 3.0f));

	engine->UpdateShaderResourceOnGPU(chData->GetConstantBuffer(0), &view, sizeof(XMMATRIX));
	engine->UpdateShaderResourceOnGPU(chData->GetConstantBuffer(1), &proj, sizeof(XMMATRIX));
}

bool TitleSceneSample::frameAction()
{
	MyGameEngine* engine = MyAccessHub::getMyGameEngine();
	GraphicsPipeLineObjectBase* normalPL = engine->GetPipelineManager()->GetPipeLineObject(L"Sprite");
	GraphicsPipeLineObjectBase* alphaPL = engine->GetPipelineManager()->GetPipeLineObject(L"AlphaSprite");

	GamePrograming3Scene* scene = static_cast<GamePrograming3Scene*>(engine->GetSceneController());
	KeyBindComponent* keyBind = static_cast<KeyBindComponent*>(scene->getKeyComponent());

	currentScene = scene->getCurrentScene();

	switch (sta)
	{
	case 0:

		if (keyBind->getCurrentInputState(InputManager::BUTTON_STATE::BUTTON_DOWN, KeyBindComponent::BUTTON_IDS::BTN_JUMP))
		{
			sta = 1;
		}
		else if ((keyBind->getCurrentInputState(InputManager::BUTTON_STATE::BUTTON_DOWN, KeyBindComponent::BUTTON_IDS::MOUSE_L)
			|| keyBind->getCurrentInputState(InputManager::BUTTON_STATE::BUTTON_DOWN, KeyBindComponent::BUTTON_IDS::MOUSE_R))
			&& currentScene != static_cast<UINT>(GAME_SCENES::TITLE))
		{
			SetNextScene(GAME_SCENES::IN_GAME);
			sta = 1;
		}
		break;

	default:
		//シーン切り替え呼び出し
		MyAccessHub::getMyGameEngine()->GetSceneController()->OrderNextScene((UINT)m_nextScene);
		//ここまで
		break;
	}

	normalPL->AddRenerObject(m_bgSp.get());
	alphaPL->AddRenerObject(m_messageSp.get());

    return true;
}

void TitleSceneSample::finishAction()
{
	GamePrograming3Scene* scene = static_cast<GamePrograming3Scene*>(MyAccessHub::getMyGameEngine()->GetSceneController());
	scene->RemoveCamera(this);
}
