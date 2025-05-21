#pragma once
#include "ecs/i_simple_system.h"

#include "numeric_types.h"

namespace Engine
{
	namespace ECS
	{
		class Prefab;
	}
}

class ServerPlayerControllerSystem : public Engine::ECS::ISimpleSystem
{
	public:
		ServerPlayerControllerSystem();

		void Execute( Engine::ECS::World& world, float32 elapsed_time ) override;

		void ConfigurePlayerControllerComponent( Engine::ECS::GameEntity& entity, const Engine::ECS::Prefab& prefab );
};
