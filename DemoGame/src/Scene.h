#pragma once

#include <SDL_image.h>

#include "entt.hpp"
#include "SpriteRendererSystem.h"
#include "ScriptSystem.h"

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
	entt::registry _registry;
	SpriteRendererSystem _spriteRendererSystem;
	ScriptSystem _scriptSystem;

	friend class GameEntity;
};
