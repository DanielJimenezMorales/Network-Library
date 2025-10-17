#pragma once
#include "ecs/i_simple_system.h"

class PlayerWeaponVisibilitySystem : public Engine::ECS::ISimpleSystem
{
	public:
		PlayerWeaponVisibilitySystem() = default;

		void Execute( Engine::ECS::World& world, float32 elapsed_time ) override;
};
