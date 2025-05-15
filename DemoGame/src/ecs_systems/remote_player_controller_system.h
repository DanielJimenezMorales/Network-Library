#pragma once
#include "ecs/i_simple_system.h"

class RemotePlayerControllerSystem : public ECS::ISimpleSystem
{
	public:
		RemotePlayerControllerSystem();

		void Execute( ECS::World& world, float32 elapsed_time ) override;
};
