#pragma once
#include "NumericTypes.h"

class EntityContainer;

class IPreTickSystem
{
public:
	virtual void PreTick(EntityContainer& entityContainer, float32 elapsedTime) const = 0;
protected:
	IPreTickSystem() {}
};
