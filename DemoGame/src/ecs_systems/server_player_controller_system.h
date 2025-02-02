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

		void Execute( ECS::EntityContainer& entity_container, float32 elapsed_time ) override;

		void ConfigurePlayerControllerComponent( GameEntity& entity, const ECS::Prefab& prefab );
};
