#pragma once
#include "ecs/i_simple_system.h"

class RemotePlayerControllerSystem : public Engine::ECS::ISimpleSystem
{
	public:
		RemotePlayerControllerSystem();

		void Execute( Engine::ECS::World& world, float32 elapsed_time ) override;
};
