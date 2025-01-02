#pragma once
#include "ecs/i_simple_system.h"

class RemotePlayerControllerSystem : public ECS::ISimpleSystem
{
	public:
		RemotePlayerControllerSystem();

		void Execute( GameEntity& entity, float32 elapsed_time ) override;
};
