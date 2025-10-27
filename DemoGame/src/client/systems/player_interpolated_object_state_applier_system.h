#pragma once
#include "ecs/i_simple_system.h"

#include "client/player_full_body_animation_applier.h"
#include "client/player_interpolated_position_and_rotation_applier.h"
#include "client/player_weapon_flip_applier.h"
#include "client/player_weapon_visibility_applier.h"

class PlayerInterpolatedObjectStateApplierSystem : public Engine::ECS::ISimpleSystem
{
	public:
		PlayerInterpolatedObjectStateApplierSystem() = default;

		void Execute( Engine::ECS::World& world, float32 elapsed_time ) override;

	private:
		PlayerFullBodyAnimationApplier _animationApplier;
		PlayerInterpolationPositionAndRotationApplier _positionAndRotationApplier;
		PlayerWeaponFlipApplier _weaponFlipApplier;
		PlayerWeaponVisibilityApplier _weaponVisibilityApplier;
};
