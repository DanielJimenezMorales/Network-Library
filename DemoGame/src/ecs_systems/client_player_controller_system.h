#pragma once
#include "ecs/i_simple_system.h"

class ClientPlayerControllerSystem : public ECS::ISimpleSystem
{
	public:
		ClientPlayerControllerSystem();

		void Execute( GameEntity& entity, float32 elapsed_time ) override;
};
