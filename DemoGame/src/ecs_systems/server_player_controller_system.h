#pragma once
#include "ecs/i_simple_system.h"

#include "numeric_types.h"

namespace ECS
{
	class Prefab;
}

class ServerPlayerControllerSystem : public ECS::ISimpleSystem
{
	public:
		ServerPlayerControllerSystem();

		void Execute( ECS::World& world, float32 elapsed_time ) override;

		void ConfigurePlayerControllerComponent( ECS::GameEntity& entity, const ECS::Prefab& prefab );
};
