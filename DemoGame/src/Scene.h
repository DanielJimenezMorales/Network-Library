#pragma once
#include "numeric_types.h"

#include <SDL_image.h>
#include <vector>

#include "SpriteRendererSystem.h"
#include "GizmoRendererSystem.h"
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

	void Update(float32 elapsedTime);
	void PreTick(float32 tickElapsedTime);
	void Tick(float32 tickElapsedTime);
	void PosTick(float32 tickElapsedTime);
	void Render(SDL_Renderer* renderer);

	void AddUpdateSystem(IUpdateSystem* system);
	void AddPreTickSystem(IPreTickSystem* system);
	void AddTickSystem(ITickSystem* system);
	void AddPosTickSystem(IPosTickSystem* system);

	GameEntity CreateGameEntity();

	template <typename T>
	GameEntity GetFirstEntityOfType();

private:
	EntityContainer _entityContainer;
	SpriteRendererSystem _spriteRendererSystem;
	GizmoRendererSystem _gizmoRendererSystem;

	std::vector<IUpdateSystem*> _updateSystems;
	std::vector<IPreTickSystem*> _preTickSystems;
	std::vector<ITickSystem*> _tickSystems;
	std::vector<IPosTickSystem*> _posTickSystems;
};

template<typename T>
inline GameEntity Scene::GetFirstEntityOfType()
{
	return _entityContainer.GetFirstEntityOfType<T>();
}
