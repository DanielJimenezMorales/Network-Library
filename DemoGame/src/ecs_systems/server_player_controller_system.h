#pragma once
#include "ecs/i_simple_system.h"

#include <cstdint>

class ServerPlayerControllerSystem : public ECS::ISimpleSystem
{
	public:
		ServerPlayerControllerSystem();

		void Execute( std::vector< GameEntity >& entities, ECS::EntityContainer& entity_container,
		              float32 elapsed_time ) override;
};
