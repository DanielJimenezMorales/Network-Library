#pragma once

class EntityContainer;

class IPosTickSystem
{
public:
	virtual void PosTick(EntityContainer& entityContainer, float32 elapsedTime) const = 0;
protected:
	IPosTickSystem() {}
};
