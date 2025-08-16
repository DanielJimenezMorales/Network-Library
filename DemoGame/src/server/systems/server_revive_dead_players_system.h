#pragma once
#include "ecs/i_simple_system.h"

class ServerReviveDeadPlayersSystem : public Engine::ECS::ISimpleSystem
{
	public:
		ServerReviveDeadPlayersSystem() = default;

		void Execute( Engine::ECS::World& world, float32 elapsed_time ) override;
};
