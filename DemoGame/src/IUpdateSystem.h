#pragma once

class EntityContainer;

class IUpdateSystem
{
public:
	virtual void Update(EntityContainer& entityContainer, float elapsedTime) const = 0;
protected:
	IUpdateSystem() {}
};
