#pragma once
#include "ecs/i_simple_system.h"

class PreTickNetworkSystem : public ECS::ISimpleSystem
{
	public:
		PreTickNetworkSystem();

		void Execute( ECS::World& world, float32 elapsed_time ) override;
};
