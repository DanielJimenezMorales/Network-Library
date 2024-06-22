#include "Scene.h"

#include "ScriptComponent.h"
#include "TransformComponent.h"
#include "GameEntity.hpp"

void Scene::Update(float elapsedTime)
{
	//Update scripts
	_scriptSystem.Update(_entityContainer, elapsedTime);
}

void Scene::Tick(float tickElapsedTime)
{
	//Tick scripts
	_scriptSystem.Tick(_entityContainer, tickElapsedTime);
}

void Scene::Render(SDL_Renderer* renderer)
{
	_spriteRendererSystem.Render(_entityContainer, renderer);
}

GameEntity Scene::CreateGameEntity()
{
	GameEntity newEntity = _entityContainer.CreateGameEntity();
	newEntity.AddComponent<TransformComponent>();

	return newEntity;
}
