#pragma once
#include "ecs/i_simple_system.h"

// TODO Remove this as it is not being used anywhere
class RemotePlayerControllerSystem : public Engine::ECS::ISimpleSystem
{
	public:
		RemotePlayerControllerSystem();

		void Execute( Engine::ECS::World& world, float32 elapsed_time ) override;
};
