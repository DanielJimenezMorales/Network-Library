#pragma once
#include "IUpdateSystem.h"
#include "ITickSystem.h"

class GameEntity;
class EntityContainer;
class ScriptComponent;

class ScriptSystem : public IUpdateSystem, public ITickSystem
{
public:
	ScriptSystem() : IUpdateSystem(), ITickSystem()
	{
	}
	void Update(EntityContainer& entityContainer, float elapsedTime) const override;
	void Tick(EntityContainer& entityContainer, float tickElapsedTime) const override;

private:
	void CreateScript(ScriptComponent& scriptComponent, const GameEntity& gameEntity) const;
};
