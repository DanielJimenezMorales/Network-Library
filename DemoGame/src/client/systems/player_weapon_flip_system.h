#pragma once
#include "ecs/i_simple_system.h"

class PlayerWeaponFlipSystem : public Engine::ECS::ISimpleSystem
{
	public:
		PlayerWeaponFlipSystem() = default;

		void Execute( Engine::ECS::World& world, float32 elapsed_time ) override;
};
