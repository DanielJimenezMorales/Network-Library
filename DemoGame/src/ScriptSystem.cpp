#include "ScriptSystem.h"
#include "ScriptComponent.h"
#include "ScriptableSystem.h"
#include "GameEntity.h"

void ScriptSystem::Update(entt::registry& registry, Scene* scene, float elapsedTime) const
{
	auto& view = registry.view<ScriptComponent>();
	for (auto entity : view)
	{
		ScriptComponent& script = view.get<ScriptComponent>(entity);
		if (!script.IsCreated())
		{
			CreateScript(script, scene, entity);
		}

		script.behaviour->Update(elapsedTime);
	}
}

void ScriptSystem::Tick(entt::registry& registry, float tickElapsedTime) const
{
}

void ScriptSystem::CreateScript(ScriptComponent& script, Scene* scene, entt::entity entity) const
{
	script.behaviour = script.CreateScript();
	script.behaviour->entity = GameEntity(entity, scene);
	script.behaviour->Create();
}
