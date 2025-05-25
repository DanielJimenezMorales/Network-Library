#pragma once
#include "ecs/i_simple_system.h"

class PreTickNetworkSystem : public Engine::ECS::ISimpleSystem
{
	public:
		PreTickNetworkSystem();

		void Execute( Engine::ECS::World& world, float32 elapsed_time ) override;
};
