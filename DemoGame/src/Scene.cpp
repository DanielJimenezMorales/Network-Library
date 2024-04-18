#include "Scene.h"

void Scene::Update(float elapsedTime)
{
}

void Scene::Render(SDL_Renderer* renderer)
{
	_spriteRendererSystem.Render(_registry, renderer);
}
