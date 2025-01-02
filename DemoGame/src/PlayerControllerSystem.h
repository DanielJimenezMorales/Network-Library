#pragma once
#include "ecs/i_simple_system.h"

class PlayerControllerSystem : public ECS::ISimpleSystem
{
	public:
		PlayerControllerSystem();

		void Execute( GameEntity& entity, float32 elapsed_time ) override;
};
