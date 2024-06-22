#pragma once

#include <SDL_image.h>
#include <vector>

#include "SpriteRendererSystem.h"
#include "EntityContainer.h"

class GameEntity;
class IUpdateSystem;
class IPreTickSystem;
class ITickSystem;
class IPosTickSystem;

class Scene
{
public:
	Scene() : _entityContainer(), _updateSystems(), _tickSystems() {}
	~Scene() {};

	void Update(float elapsedTime);
	void PreTick(float tickElapsedTime);
	void Tick(float tickElapsedTime);
	void PosTick(float tickElapsedTime);
	void Render(SDL_Renderer* renderer);

	void AddUpdateSystem(IUpdateSystem* system);
	void AddPreTickSystem(IPreTickSystem* system);
	void AddTickSystem(ITickSystem* system);
	void AddPosTickSystem(IPosTickSystem* system);

	GameEntity CreateGameEntity();

private:
	EntityContainer _entityContainer;
	SpriteRendererSystem _spriteRendererSystem;

	std::vector<IUpdateSystem*> _updateSystems;
	std::vector<IPreTickSystem*> _preTickSystems;
	std::vector<ITickSystem*> _tickSystems;
	std::vector<IPosTickSystem*> _posTickSystems;
};
