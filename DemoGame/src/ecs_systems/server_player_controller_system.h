#pragma once
#include "ecs/i_simple_system.h"

#include <cstdint>

class ServerPlayerControllerSystem : public ECS::ISimpleSystem
{
	public:
		ServerPlayerControllerSystem();

		void Execute( GameEntity& entity, float32 elapsed_time ) override;
};
