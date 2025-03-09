#pragma once
#include "ecs/i_simple_system.h"

namespace ECS
{
	class Prefab;
	class World;
	class GameEntity;
}

class TemporaryLifetimeObjectsSystem : public ECS::ISimpleSystem
{
	public:
		TemporaryLifetimeObjectsSystem( ECS::World* world );

		void Execute( ECS::EntityContainer& entity_container, float32 elapsed_time ) override;

		void ConfigureTemporaryLifetimeComponent( ECS::GameEntity& entity, const ECS::Prefab& prefab );

	private:
		ECS::World* _world;
};
