#pragma once

class EntityContainer;

class ITickSystem
{
public:
	virtual void Tick(EntityContainer& entityContainer, float elapsedTime) const = 0;
protected:
	ITickSystem() {}
};
