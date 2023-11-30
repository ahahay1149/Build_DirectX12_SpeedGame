#pragma once
#include <GameObject.h>
#include "FBXCharacterData.h"


class SkyDomeComponent : public GameComponent
{
private:
	CharacterData* centerCharacter;

public:
	void initAction() override;
	bool frameAction() override;
	void finishAction() override;

	void setCenterCharacter(CharacterData* target);
};
