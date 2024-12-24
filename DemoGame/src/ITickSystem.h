#pragma once
#include "numeric_types.h"

class EntityContainer;

class ITickSystem
{
public:
	virtual void Tick(EntityContainer& entityContainer, float32 elapsedTime) const = 0;
protected:
	ITickSystem() {}
};
