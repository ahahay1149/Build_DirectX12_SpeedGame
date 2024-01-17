#include "MovingPlatform.h"

void MovingPlatform::hitReaction(GameObject* targetGo, HitAreaBase* hit)
{
}

void MovingPlatform::initAction()
{
	CharacterData* chd = getGameObject()->getCharacterData();

	hRad = 0.0f;
	rotDeg = 0.0f;

	yPos = chd->getPosition().y;	//中央のX

	sta = 0;
}

bool MovingPlatform::frameAction()
{
	const float addH = 0.002f * XM_PI;
	const float addRot = 1.0f;
	const float moveLength = 10.0f;

	bool activeCheck = isActive();
	if (activeCheck == true)
	{
		CharacterData* chd = getGameObject()->getCharacterData();

		//X移動
		hRad += addH;
		hRad = addH > XM_PI ? hRad - 2.0f * XM_PI : hRad < -XM_PI ? hRad + 2.0f * XM_PI : hRad;

		XMFLOAT3 pos = chd->getPosition();

		chd->setPosition(pos.x, yPos + moveLength + moveLength * sin(hRad), pos.z);

		//回転
		rotDeg += addRot;
		rotDeg = rotDeg > 180.0f ? rotDeg - 360.0f : rotDeg < -180.0f ? rotDeg + 360.0f : rotDeg;

		chd->setRotation(0, rotDeg, 0);
	}

	return true;
}

void MovingPlatform::finishAction()
{
}
