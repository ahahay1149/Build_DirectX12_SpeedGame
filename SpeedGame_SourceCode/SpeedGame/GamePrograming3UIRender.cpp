#include "GamePrograming3UIRender.h"

#include <MyAccessHub.h>
#include <D3D12Helper.h>

#include "GamePrograming3Scene.h"
#include "GamePrograming3Enum.h"
#include "HeartItemComponent.h"
#include "UnityChanPlayer.h"

int GamePrograming3UIRender::MakeSpriteString(int startIndex, float ltX, float ltY, float width, float height, const char* str)
{
	int count = startIndex;

	while (*str != '\0')
	{
		if (std::find(m_chList, m_chEnd, *str) != m_chEnd)
		{
			m_sprites[count]->SetSpritePattern(0, width, height, m_fontMap[*str]);
			m_sprites[count]->setSpriteIndex(0);

			m_sprites[count]->setPosition(ltX, ltY, 0.0f);
			count++;
		}

		ltX += width;

		str++;
	}

	return count;
}

void GamePrograming3UIRender::initAction()
{
	MyGameEngine* engine = MyAccessHub::getMyGameEngine();
	CharacterData* chData = getGameObject()->getCharacterData();

	engine->InitCameraConstantBuffer(chData);

	chData->setPosition(0.0f, 0.0f, 0.0f);

	XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);		//視点（カメラ）座標
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);		//フォーカスする（カメラが向く）座標
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);		//カメラの上方向単位ベクトル（カメラのロール軸）
	XMMATRIX view = XMMatrixTranspose(MakeViewMatix(Eye, At, Up));
	XMMATRIX proj = XMMatrixTranspose(MakeOrthographicPrjectionMatrix(engine->GetWidth(), engine->GetHeight(), 0.01f, 3.0f));


	engine->UpdateShaderResourceOnGPU(chData->GetConstantBuffer(0), &view, sizeof(XMMATRIX));
	engine->UpdateShaderResourceOnGPU(chData->GetConstantBuffer(1), &proj, sizeof(XMMATRIX));


	m_spriteCount = 50;

	SpriteCharacter* spc;

	for (int i = 0; i < m_spriteCount; i++)
	{
		spc = new SpriteCharacter();

		spc->setTextureId(L"HUDTexture");
		spc->SetCameraLabel(L"HUDCamera");

		spc->setColor(1,1,1,1);

		//UI用のパイプラインは別にすべきなんだけども、記述量を減らしたいので使いまわし
		spc->SetGraphicsPipeLine(L"Sprite");

		m_sprites.push_back(std::unique_ptr<SpriteCharacter>(spc));
	}

	//FontMap
	Texture2DContainer* tex = engine->GetTextureManager()->GetTexture(L"HUDTexture");

	m_fontMap.reserve(strlen(m_chList));

	m_chEnd = m_chList + strlen(m_chList) * sizeof(m_chList[0]);

	int index = 0;
	float invW = 1.0f / tex->fWidth;
	float invH = 1.0f / tex->fHeight;

	float x = 0.0f;
	float y = 0.0f;
	float w = 32.0f;	//フォントサイズ
	float h = 56.0f;	//等幅フォントだと計算簡単
	while (m_chList[index] != '\0')
	{
		XMFLOAT4 r = {x * invW, y * invH, w * invW, h * invH};
		m_fontMap[m_chList[index]] = r;
		x += 32.0f;

		if (x >= tex->fWidth)
		{
			x = 0.0f;
			y += h;
		}

		index++;
	}
}

bool GamePrograming3UIRender::frameAction()
{
	MyGameEngine* engine = MyAccessHub::getMyGameEngine();
	GraphicsPipeLineObjectBase* pipeLine = engine->GetPipelineManager()->GetPipeLineObject(L"Sprite");

	GamePrograming3Scene* scene = GamePrograming3Scene::getScene();

	int count = 0;
	float x = -480.0f + 30.0f;	//中心0,0 w960 h540
	float y = 270.0f - 30.0f;

	//開始前のカウントダウンのUI制御
	startCount -= 0.01666f;
	if (startCount > 3.0f)
	{
		sprintf_s(countText, "  %d", 3);
	}
	else if (startCount > 2.0f)
	{
		sprintf_s(countText, "  %d", 2);
	}
	else if (startCount > 1.0f)
	{
		sprintf_s(countText, "  %d", 1);
	}
	else if (startCount > 0.0f)
	{
		sprintf_s(countText, "Start!");
	}
	else
	{
		sprintf_s(countText, "");

		//ゲーム中のカウントダウンUI制御
		timerCount -= 0.01666f;
		sprintf_s(timerText, "Time %.3f", timerCount);
		//共通のタイマーシステムまだ作ってないので、UIの遷移もここでやってしまう
		if (timerCount < 0)
		{
			engine->GetSceneController()->OrderNextScene((UINT)GAME_SCENES::GAME_OVER);
		}
	}

	//ハートアイテムのUI制御。取得数カウント
	sprintf_s(itemText, "Item %d / 5", heartItemCount);

	count = MakeSpriteString(count, x, y, 24, 24, itemText);
	count = MakeSpriteString(count, x, y - 60, 24, 24, timerText);

	count = MakeSpriteString(count, -100, 0, 48, 48, countText);

	if (heartItemCount >= 5)
	{
		scene->setClearCount(timerCount);
		engine->GetSceneController()->OrderNextScene((UINT)GAME_SCENES::GAME_CLEAR);
	}

	//使ったSpriteCharacterだけをパイプラインに登録
	for (int i = 0; i < count; i++)
	{
		pipeLine->AddRenerObject(m_sprites[i].get());
	}

	return true;
}

void GamePrograming3UIRender::finishAction()
{
	m_sprites.clear();

	GamePrograming3Scene* scene = static_cast<GamePrograming3Scene*>(MyAccessHub::getMyGameEngine()->GetSceneController());
	scene->RemoveCamera(this);
}

void GamePrograming3UIRender::plusHeartItemCount()
{
	heartItemCount++;
}