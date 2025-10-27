#include "player_interpolated_object_state_applier.h"

#include "ecs/world.h"
#include "ecs/game_entity.hpp"

#include "transform/transform_component.h"
#include "transform/transform_hierarchy_helper_functions.h"

#include "client/components/player_interpolated_state_component.h"
#include "client/components/player_aim_component.h"
#include "client/components/player_body_animation_tag_component.h"
#include "client/components/player_visual_weapon_tag_component.h"

void PlayerInterpolatedObjectStateApplierSystem::Execute( Engine::ECS::World& world, float32 elapsed_time )
{
	const Engine::TransformComponentProxy transformProxy;

	std::vector< Engine::ECS::GameEntity > entities = world.GetEntitiesOfType< PlayerInterpolatedStateComponent >();

	for ( auto it = entities.begin(); it != entities.end(); ++it )
	{
		// Get children entities
		Engine::ECS::GameEntity playerAimEntity = it->GetFirstChildWithComponent< PlayerAimComponent >();
		Engine::ECS::GameEntity playerVisualWeaponEntity =
		    it->GetFirstChildWithComponent< PlayerVisualWeaponTagComponent >();
		Engine::ECS::GameEntity playerFullBodyEntity =
		    it->GetFirstChildWithComponent< PlayerBodyAnimationTagComponent >();

		// Get interpolated state
		const PlayerInterpolatedStateComponent& interpolatedStateComponent =
		    it->GetComponent< PlayerInterpolatedStateComponent >();
		const PlayerInterpolatedState& interpolatedState = interpolatedStateComponent.state;

		// Apply interpolated state
		_animationApplier.Execute( playerFullBodyEntity, interpolatedState, elapsed_time );
		_positionAndRotationApplier.Execute( *it, playerAimEntity, interpolatedState, elapsed_time );
		_weaponFlipApplier.Execute( playerVisualWeaponEntity, interpolatedState, elapsed_time );
		_weaponVisibilityApplier.Execute( playerVisualWeaponEntity, interpolatedState, elapsed_time );
	}
}
