#pragma once
#include "numeric_types.h"

class EntityContainer;

class IUpdateSystem
{
public:
	virtual void Update(EntityContainer& entityContainer, float32 elapsedTime) const = 0;
protected:
	IUpdateSystem() {}
};
