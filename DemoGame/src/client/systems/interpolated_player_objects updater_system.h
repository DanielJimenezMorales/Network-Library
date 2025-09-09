#pragma once
#include "ecs/i_simple_system.h"
// TODO Fix filename, there is a space character that should be a '_'
class InterpolatedPlayerObjectUpdaterSystem : public Engine::ECS::ISimpleSystem
{
	public:
		InterpolatedPlayerObjectUpdaterSystem();

		void Execute( Engine::ECS::World& world, float32 elapsed_time ) override;
};
