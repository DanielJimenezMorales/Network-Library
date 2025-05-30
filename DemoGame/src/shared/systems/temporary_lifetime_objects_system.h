#pragma once
#include "ecs/i_simple_system.h"

namespace Engine
{
	namespace ECS
	{
		class Prefab;
		class GameEntity;
	}
}

class TemporaryLifetimeObjectsSystem : public Engine::ECS::ISimpleSystem
{
	public:
		TemporaryLifetimeObjectsSystem();

		void Execute( Engine::ECS::World& world, float32 elapsed_time ) override;

		void ConfigureTemporaryLifetimeComponent( Engine::ECS::GameEntity& entity, const Engine::ECS::Prefab& prefab );
};
