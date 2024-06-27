#pragma once

class EntityContainer;

class IPreTickSystem
{
public:
	virtual void PreTick(EntityContainer& entityContainer, float elapsedTime) const = 0;
protected:
	IPreTickSystem() {}
};
