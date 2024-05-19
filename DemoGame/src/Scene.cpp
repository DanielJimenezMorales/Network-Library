#include "Scene.h"

#include "ScriptComponent.h"
#include "TransformComponent.h"
#include "GameEntity.h"

void Scene::Update(float elapsedTime)
{
	//Update scripts
	_scriptSystem.Update(_registry, this, elapsedTime);
}

void Scene::Tick(float tickElapsedTime)
{
	//Tick scripts
	_scriptSystem.Tick(_registry, tickElapsedTime);
}

void Scene::Render(SDL_Renderer* renderer)
{
	_spriteRendererSystem.Render(_registry, renderer);
}

GameEntity Scene::CreateGameEntity()
{
	entt::entity ecsId = _registry.create();
	GameEntity newEntity = GameEntity(ecsId, this);
	newEntity.AddComponent<TransformComponent>();

	return newEntity;
}
