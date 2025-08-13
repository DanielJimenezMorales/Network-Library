#pragma once
#include "ecs/i_simple_system.h"

class ServerHitRegistrationSystem : public Engine::ECS::ISimpleSystem
{
	public:
		ServerHitRegistrationSystem() = default;

		void Execute( Engine::ECS::World& world, float32 elapsed_time ) override;
};
