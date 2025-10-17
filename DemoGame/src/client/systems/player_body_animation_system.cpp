#include "player_body_animation_system.h"

#include "logger.h"
#include "math_utils.h"

#include "ecs/world.h"
#include "ecs/game_entity.hpp"

#include "components/transform_component.h"
#include "components/animation_component.h"

#include "transform/transform_hierarchy_helper_functions.h"

#include "shared/components/player_controller_component.h"

#include "client/components/player_body_animation_tag_component.h"
#include "client/components/ghost_object_component.h"

#include <cassert>
#include <string>

static std::string GetAnimationNameBasedOnState( bool is_walking, const Vec2f& forward_direction )
{
	std::string newAnimationName;

	// Get angle in radians (-π to π)
	const float32 angleRadians = std::atan2( forward_direction.Y(), forward_direction.X() );

	// Convert to degrees for easier sector checks
	float32 angleDegrees = angleRadians * 180.f / Engine::PI;

	// Normalize degrees to [0, 360)
	if ( angleDegrees < 0 )
		angleDegrees += 360.f;

	// Determine sector (8-way, each 45° wide)
	if ( angleDegrees >= 337.5f || angleDegrees < 22.5f )
		newAnimationName = is_walking ? "WALK_RIGHT" : "IDLE_RIGHT";
	else if ( angleDegrees < 67.5f )
		newAnimationName = is_walking ? "WALK_BACK_RIGHT" : "IDLE_BACK_RIGHT";
	else if ( angleDegrees < 112.5f )
		newAnimationName = is_walking ? "WALK_BACK" : "IDLE_BACK";
	else if ( angleDegrees < 157.5f )
		newAnimationName = is_walking ? "WALK_BACK_LEFT" : "IDLE_BACK_LEFT";
	else if ( angleDegrees < 202.5f )
		newAnimationName = is_walking ? "WALK_LEFT" : "IDLE_LEFT";
	else if ( angleDegrees < 247.5f )
		newAnimationName = is_walking ? "WALK_FRONT_LEFT" : "IDLE_FRONT_LEFT";
	else if ( angleDegrees < 292.5f )
		newAnimationName = is_walking ? "WALK_FRONT" : "IDLE_FRONT";
	else // < 337.5f
		newAnimationName = is_walking ? "WALK_FRONT_RIGHT" : "IDLE_FRONT_RIGHT";

	return newAnimationName;
}

static void ApplyAnimationName( Engine::AnimationComponent& animation, const std::string& targetAnimationName )
{
	if ( animation.currentAnimation->name != targetAnimationName )
	{
		if ( animation.animations.find( targetAnimationName ) != animation.animations.end() )
		{
			const Engine::AnimationClip* newAnimationClip = &animation.animations[ targetAnimationName ];
			animation.currentAnimation = newAnimationClip;
			animation.currentFrame = 0;
			animation.timeAccumulator = 0.f;
		}
		else
		{
			LOG_ERROR( "[%s] Animation Clip with name %s couldn't be found", THIS_FUNCTION_NAME,
			           targetAnimationName.c_str() );
		}
	}
}

static bool TryGetIdleEquivalentAnimationName( const std::string& animation_name, std::string& idle_equivalent_name )
{
	bool foundIdleEquivalent = true;

	if ( animation_name == "WALK_RIGHT" )
		idle_equivalent_name = "IDLE_RIGHT";
	else if ( animation_name == "WALK_BACK_RIGHT" )
		idle_equivalent_name = "IDLE_BACK_RIGHT";
	else if ( animation_name == "WALK_BACK" )
		idle_equivalent_name = "IDLE_BACK";
	else if ( animation_name == "WALK_BACK_LEFT" )
		idle_equivalent_name = "IDLE_BACK_LEFT";
	else if ( animation_name == "WALK_LEFT" )
		idle_equivalent_name = "IDLE_LEFT";
	else if ( animation_name == "WALK_FRONT_LEFT" )
		idle_equivalent_name = "IDLE_FRONT_LEFT";
	else if ( animation_name == "WALK_FRONT" )
		idle_equivalent_name = "IDLE_FRONT";
	else if ( animation_name == "WALK_FRONT_RIGHT" )
		idle_equivalent_name = "IDLE_FRONT_RIGHT";
	else
		foundIdleEquivalent = false;

	return foundIdleEquivalent;
}

static void ApplyIdleEquivalentAnimation( Engine::AnimationComponent& animation )
{
	if ( animation.currentAnimation != nullptr )
	{
		std::string idleEquivalentName;
		if ( TryGetIdleEquivalentAnimationName( animation.currentAnimation->name, idleEquivalentName ) )
		{
			ApplyAnimationName( animation, idleEquivalentName );
		}
	}
}

static void UpdateAnimationComponent( Engine::ECS::GameEntity& entity,
                                      const PlayerControllerComponent& player_controller,
                                      const Vec2f& weapon_forward_direction )
{
	Engine::AnimationComponent& animation = entity.GetComponent< Engine::AnimationComponent >();

	// If the player is aiming or walking, update the animation based on the direction
	if ( player_controller.isAiming || player_controller.isWalking )
	{
		// When aiming, use the weapon's forward direction for animation selection, otherwise use the movement
		// direction.
		const Vec2f forwardDirection =
		    ( player_controller.isAiming ) ? weapon_forward_direction : player_controller.movementDirection;

		const std::string targetAnimationName =
		    GetAnimationNameBasedOnState( player_controller.isWalking, forwardDirection );
		ApplyAnimationName( animation, targetAnimationName );
	}
	// Otherwise, transition to the idle animation equivalent if applicable
	else
	{
		ApplyIdleEquivalentAnimation( animation );
	}
}

void PlayerBodyAnimationSystem::Execute( Engine::ECS::World& world, float32 elapsed_time )
{
	const Engine::TransformComponentProxy transformComponentProxy;

	std::vector< Engine::ECS::GameEntity > playerEntities =
	    world.GetEntitiesOfType< PlayerBodyAnimationTagComponent >();
	for ( auto it = playerEntities.begin(); it != playerEntities.end(); ++it )
	{
		const Engine::TransformComponent& transform = it->GetComponent< Engine::TransformComponent >();
		const Engine::ECS::GameEntity parentEntity = transformComponentProxy.GetParent( transform );
		assert( parentEntity.IsValid() );

		const GhostObjectComponent& ghostObject = parentEntity.GetComponent< GhostObjectComponent >();
		const Engine::ECS::GameEntity& ghostEntity = ghostObject.entity;
		assert( ghostEntity.IsValid() );

		const Engine::TransformComponent& parentTransform = parentEntity.GetComponent< Engine::TransformComponent >();
		const std::vector< Engine::ECS::GameEntity > children = transformComponentProxy.GetChildren( parentTransform );
		auto childIt = children.cbegin();
		for ( ; childIt != children.cend(); ++childIt )
		{
			if ( *childIt != *it )
			{
				break;
			}
		}

		const PlayerControllerComponent& playerController = ghostEntity.GetComponent< PlayerControllerComponent >();
		const Engine::TransformComponent& rotationEntityTransform =
		    childIt->GetComponent< Engine::TransformComponent >();

		UpdateAnimationComponent( *it, playerController,
		                          transformComponentProxy.GetForwardVector( rotationEntityTransform ) );
	}
}
