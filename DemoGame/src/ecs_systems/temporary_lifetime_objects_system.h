#pragma once
#include "ecs/i_simple_system.h"

namespace ECS
{
	class Prefab;
	class GameEntity;
}

class TemporaryLifetimeObjectsSystem : public ECS::ISimpleSystem
{
	public:
		TemporaryLifetimeObjectsSystem();

		void Execute( ECS::World& world, float32 elapsed_time ) override;

		void ConfigureTemporaryLifetimeComponent( ECS::GameEntity& entity, const ECS::Prefab& prefab );
};
