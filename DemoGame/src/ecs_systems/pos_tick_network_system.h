#pragma once
#include "ecs/i_simple_system.h"

class PosTickNetworkSystem : public Engine::ECS::ISimpleSystem
{
	public:
		PosTickNetworkSystem();

		void Execute( Engine::ECS::World& world, float32 elapsed_time ) override;
};
