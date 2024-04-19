#pragma once
#include "entt.hpp"

class Scene;
class ScriptComponent;

class ScriptSystem
{
public:
	void Update(entt::registry& registry, Scene* scene, float elapsedTime) const;
	void Tick(entt::registry& registry, float tickElapsedTime) const;

private:
	void CreateScript(ScriptComponent& scriptComponent, Scene* scene, entt::entity) const;
};
