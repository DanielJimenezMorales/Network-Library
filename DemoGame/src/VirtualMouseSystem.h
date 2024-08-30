#pragma once
#include "IUpdateSystem.h"

class VirtualMouseSystem : public IUpdateSystem
{
public:
	void Update(EntityContainer& entityContainer, float32 elapsedTime) const override;
};
