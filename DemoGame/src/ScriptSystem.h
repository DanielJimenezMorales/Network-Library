#pragma once

class GameEntity;
class EntityContainer;
class ScriptComponent;

class ScriptSystem
{
public:
	void Update(EntityContainer& entityContainer, float elapsedTime) const;
	void Tick(EntityContainer& entityContainer, float tickElapsedTime) const;

private:
	void CreateScript(ScriptComponent& scriptComponent, const GameEntity& gameEntity) const;
};
