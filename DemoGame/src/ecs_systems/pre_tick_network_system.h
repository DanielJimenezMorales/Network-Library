#pragma once
#include "ecs/i_simple_system.h"

class PreTickNetworkSystem : public ECS::ISimpleSystem
{
	public:
		PreTickNetworkSystem();

		void Execute( std::vector< GameEntity >& entities, ECS::EntityContainer& entity_container,
		              float32 elapsed_time ) override;
};
