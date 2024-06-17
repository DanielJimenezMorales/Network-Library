#pragma once

#include <SDL_image.h>
#include <cassert>

#include "SpriteRendererSystem.h"
#include "ScriptSystem.h"
#include "EntityContainer.h"

class GameEntity;

class Scene
{
public:
	Scene() {};
	~Scene() {};

	void Update(float elapsedTime);
	void Tick(float tickElapsedTime);
	void Render(SDL_Renderer* renderer);

	GameEntity CreateGameEntity();

private:
	EntityContainer _entityContainer;
	SpriteRendererSystem _spriteRendererSystem;
	ScriptSystem _scriptSystem;

	friend class GameEntity;
};
