#include "ScriptSystem.h"
#include "ScriptComponent.h"
#include "ScriptableSystem.h"
#include "GameEntity.hpp"
#include "EntityContainer.h"

void ScriptSystem::Update(EntityContainer& entityContainer, float elapsedTime) const
{
	std::vector<GameEntity> entitiesWithScript = entityContainer.GetEntitiesOfType<ScriptComponent>();
	auto it = entitiesWithScript.begin();
	for (; it != entitiesWithScript.end(); ++it)
	{
		ScriptComponent& script = it->GetComponent<ScriptComponent>();
		if (!script.isCreated)
		{
			CreateScript(script, *it);
		}

		script.behaviour->Update(elapsedTime);
	}
}

void ScriptSystem::Tick(EntityContainer& entityContainer, float tickElapsedTime) const
{
	std::vector<GameEntity> entitiesWithScript = entityContainer.GetEntitiesOfType<ScriptComponent>();
	auto it = entitiesWithScript.begin();
	for (; it != entitiesWithScript.end(); ++it)
	{
		ScriptComponent& script = it->GetComponent<ScriptComponent>();
		if (!script.isCreated)
		{
			continue;
		}

		script.behaviour->Tick(tickElapsedTime);
	}
}

void ScriptSystem::CreateScript(ScriptComponent& script, const GameEntity& gameEntity) const
{
	script.behaviour->entity = gameEntity;
	script.behaviour->Create();
	script.isCreated = true;
}
