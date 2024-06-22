#pragma once

class EntityContainer;

class IPosTickSystem
{
public:
	virtual void PosTick(EntityContainer& entityContainer, float elapsedTime) const = 0;
protected:
	IPosTickSystem() {}
};
