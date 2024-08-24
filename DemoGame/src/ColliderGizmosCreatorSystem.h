#pragma once
#include "IUpdateSystem.h"

class ColliderGizmosCreatorSystem : public IUpdateSystem
{
public:
	void Update(EntityContainer& entityContainer, float elapsedTime) const;
};
