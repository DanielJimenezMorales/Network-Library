#pragma once
#include "ecs/i_simple_system.h"

class CrosshairFollowMouseSystem : public Engine::ECS::ISimpleSystem
{
	public:
		void Execute( Engine::ECS::World& world, float32 elapsed_time ) override;
};
