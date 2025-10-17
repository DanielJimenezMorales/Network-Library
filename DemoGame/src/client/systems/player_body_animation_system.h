#pragma once
#include "ecs/i_simple_system.h"

class PlayerBodyAnimationSystem : public Engine::ECS::ISimpleSystem
{
	public:
		PlayerBodyAnimationSystem() = default;

		void Execute( Engine::ECS::World& world, float32 elapsed_time ) override;
};
