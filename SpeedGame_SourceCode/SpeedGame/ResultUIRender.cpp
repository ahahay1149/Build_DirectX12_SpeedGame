#include "ResultUIRender.h"

#include <MyAccessHub.h>
#include <D3D12Helper.h>
#include <stdio.h>

#include "GamePrograming3Scene.h"
#include "GamePrograming3Enum.h"
#include "HeartItemComponent.h"
#include "UnityChanPlayer.h"

void ResultUIRender::initAction()
{
	UIRenderBase::initAction();
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
	UIRenderBase::finishAction();
}