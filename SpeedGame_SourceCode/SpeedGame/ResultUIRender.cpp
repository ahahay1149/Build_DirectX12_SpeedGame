#include "ResultUIRender.h"

#include <MyAccessHub.h>
#include <D3D12Helper.h>
#include <stdio.h>

#include "GamePrograming3Scene.h"
#include "GamePrograming3Enum.h"
#include "HeartItemComponent.h"
#include "UnityChanPlayer.h"

int ResultUIRender::MakeSpriteString(int startIndex, float ltX, float ltY, float width, float height, const char* str)
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

void ResultUIRender::initAction()
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

	//文字数上限(これを超えるとエラー)
	m_spriteCount = 100;

	SpriteCharacter* spc;

	for (int i = 0; i < m_spriteCount; i++)
	{
		spc = new SpriteCharacter();

		spc->setTextureId(L"HUDTexture");
		spc->SetCameraLabel(L"HUDCamera");

		spc->setColor(1, 1, 1, 1);

		//UI用のパイプラインは別にすべきなんだけども、記述量を減らしたいので使いまわし。
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
		XMFLOAT4 r = { x * invW, y * invH, w * invW, h * invH };
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

bool ResultUIRender::frameAction()
{
	MyGameEngine* engine = MyAccessHub::getMyGameEngine();
	GamePrograming3Scene* scene = GamePrograming3Scene::getScene();
	GraphicsPipeLineObjectBase* pipeLine = engine->GetPipelineManager()->GetPipeLineObject(L"Sprite");

	float x = -480.0f + 30.0f;	//中心0,0 w960 h540
	float y = 270.0f - 30.0f;

	int count = 0;

	clearCount = scene->getClearCount();
	bestScore = scene->getBestScore();

	//現在のタイムとベストタイムを比較し、タイムの上書きを行う
	if (clearCount > bestScore)
	{
		bestScore = clearCount;
		scene->setBestScore(bestScore);
	}

	//現在のタイム
	sprintf_s(clearCountText, "Time [%.3f]", clearCount);
	//ベストタイム
	sprintf_s(bestScoreText, "Best Time <%.3f>", bestScore);

	count = MakeSpriteString(count, x + 300, y - 320, 24, 24, clearCountText);
	count = MakeSpriteString(count, x + 180, y - 350, 24, 24, bestScoreText);
	count = MakeSpriteString(count, -215, -180, 24, 24, retryText);
	count = MakeSpriteString(count, -170, -210, 24, 24, titleText);
	//ベストスコアなら通知を1秒間隔で点滅させる
	if (clearCount >= bestScore)
	{
		scoreUpdateTimer += 1;
		if ((scoreUpdateTimer / 60) % 2 == 0)
		{
			sprintf_s(scoreUpdateText, "Score Update!");
		}
		else
		{
			sprintf_s(scoreUpdateText, "");
		}
		count = MakeSpriteString(count, x + 630, y - 350, 18, 18, scoreUpdateText);
	}

	//使ったSpriteCharacterだけをパイプラインに登録
	for (int i = 0; i < count; i++)
	{
		pipeLine->AddRenerObject(m_sprites[i].get());
	}

	return true;
}

void ResultUIRender::finishAction()
{
	m_sprites.clear();

	GamePrograming3Scene* scene = static_cast<GamePrograming3Scene*>(MyAccessHub::getMyGameEngine()->GetSceneController());
	scene->RemoveCamera(this);
}