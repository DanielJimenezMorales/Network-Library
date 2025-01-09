#pragma once
#include "ecs/i_simple_system.h"

class RemotePlayerControllerSystem : public ECS::ISimpleSystem
{
	public:
		RemotePlayerControllerSystem();

		void Execute( std::vector< GameEntity >& entities, ECS::EntityContainer& entity_container,
		              float32 elapsed_time ) override;
};
