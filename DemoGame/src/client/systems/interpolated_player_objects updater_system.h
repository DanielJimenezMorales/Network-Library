#pragma once
#include "ecs/i_simple_system.h"

class InterpolatedPlayerObjectUpdaterSystem : public Engine::ECS::ISimpleSystem
{
	public:
		InterpolatedPlayerObjectUpdaterSystem();

		void Execute( Engine::ECS::World& world, float32 elapsed_time ) override;
};
