#pragma once
#include <cassert>

#include "entt.hpp"
#include "Scene.h"

class GameEntity
{
public:
	GameEntity() = default;
	GameEntity(entt::entity enttId, Scene* scene) : _ecsEntityId(enttId), _scene(scene) {};
	GameEntity(const GameEntity& other) = default;
	~GameEntity() = default;

	template <typename T, typename... Params>
	T& AddComponent(Params&&... params);

	template <typename T>
	bool HasComponent();

	template <typename T>
	T& GetComponent();

	template <typename T>
	void RemoveComponent();

private:
	entt::entity _ecsEntityId;
	Scene* _scene;
};

template<typename T, typename ...Params>
inline T& GameEntity::AddComponent(Params && ...params)
{
	assert(!HasComponent<T>());
	return _scene->_registry.emplace<T>(_ecsEntityId, std::forward<Params>(params)...);
};

template<typename T>
inline bool GameEntity::HasComponent()
{
	return _scene->_registry.all_of<T>(_ecsEntityId);
};

template<typename T>
inline T& GameEntity::GetComponent()
{
	assert(HasComponent<T>());
	return _scene->_registry.get<T>(_ecsEntityId);
};

template<typename T>
inline void GameEntity::RemoveComponent()
{
	assert(HasComponent<T>());
	_scene->_registry.remove<T>(_ecsEntityId);
};
