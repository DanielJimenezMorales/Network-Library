#include "local_player_interpolated_object_state_updater_system.h"

#include <vector>

#include "ecs/world.h"
#include "ecs/game_entity.hpp"

#include "transform/transform_component.h"
#include "transform/transform_hierarchy_helper_functions.h"

#include "client/components/player_interpolated_state_component.h"
#include "client/components/ghost_object_component.h"
#include "client/components/interpolated_object_component.h"

#include "shared/components/player_controller_component.h"
#include "shared/components/network_entity_component.h"

static float32 GetMinimumDistanceBetweenAngles( float32 a, float32 b )
{
	const float32 delta = fmodf( b - a + 540.0f, 360.0f ) - 180.0f;
	return fabs( delta );
}

static bool IsSnapToGhostRequired( const Engine::TransformComponent& ghost,
                                   const Engine::TransformComponent& interpolated,
                                   const InterpolatedObjectComponent& interpolation_config )
{
	const Engine::TransformComponentProxy transformComponentProxy;
	return Vec2f::GetSquareDistance( transformComponentProxy.GetGlobalPosition( ghost ),
	                                 transformComponentProxy.GetGlobalPosition( interpolated ) ) >=
	           interpolation_config.squaredSnapToGhostPositionDistanceThreshold ||
	       GetMinimumDistanceBetweenAngles( transformComponentProxy.GetGlobalRotation( ghost ),
	                                        transformComponentProxy.GetGlobalRotation( interpolated ) ) >=
	           interpolation_config.snapToGhostOrientationThreshold;
}

static float32 InterpolateFloat32( float32 initial, float32 target, float32 max_distance_delta )
{
	const float32 delta = target - initial;
	if ( fabs( delta ) <= max_distance_delta )
	{
		return target;
	}

	const float32 deltaSign = delta > 0.f ? 1.0f : -1.0f;
	return initial + ( deltaSign * max_distance_delta );
}

static float32 MoveTowardsAngle( float32 initial, float32 target, float32 max_distance_delta )
{
	const float32 delta = fmodf( target - initial + 540.0f, 360.0f ) - 180.0f;

	if ( fabs( delta ) <= max_distance_delta )
	{
		return target;
	}

	const float32 deltaSign = delta > 0.f ? 1.0f : -1.0f;
	return initial + ( deltaSign * max_distance_delta );
}

void LocalPlayerInterpolatedObjectStateUpdaterSystem::Execute( Engine::ECS::World& world, float32 elapsed_time )
{
	Engine::TransformComponentProxy transformComponentProxy;

	std::vector< Engine::ECS::GameEntity > entities = world.GetEntitiesOfType< PlayerInterpolatedStateComponent >();

	for ( auto it = entities.begin(); it != entities.end(); ++it )
	{
		if ( it->HasComponent< NetworkEntityComponent >() )
		{
			continue;
		}

		PlayerInterpolatedStateComponent& interpolatedStateComponent =
		    it->GetComponent< PlayerInterpolatedStateComponent >();

		// TODO Choose interpolation update algorithm depending on the type of interpolation we need
		const InterpolatedObjectComponent& interpolatedObject = it->GetComponent< InterpolatedObjectComponent >();
		const GhostObjectComponent& ghostObject = it->GetComponent< GhostObjectComponent >();

		if ( !ghostObject.entity.IsValid() )
		{
			LOG_WARNING( "The Ghost object associated with an interpolated object is not valid. Skipping it." );
			continue;
		}

		const PlayerControllerComponent& playerControllerComponent =
		    ghostObject.entity.GetComponent< PlayerControllerComponent >();

		PlayerInterpolatedState newState;
		newState.isAiming = playerControllerComponent.state.isAiming;
		newState.isWalking = playerControllerComponent.state.isWalking;
		newState.movementDirection = playerControllerComponent.state.movementDirection;

		const Engine::TransformComponent& ghostTransform =
		    ghostObject.entity.GetComponent< Engine::TransformComponent >();

		Engine::TransformComponent& interpolatedTransform = it->GetComponent< Engine::TransformComponent >();

		Vec2f finalPosition;
		float32 finalRotationAngle;
		if ( IsSnapToGhostRequired( ghostTransform, interpolatedTransform, interpolatedObject ) )
		{
			finalPosition = transformComponentProxy.GetGlobalPosition( ghostTransform );
			finalRotationAngle = transformComponentProxy.GetGlobalRotation( ghostTransform );
		}
		else
		{
			const float32 positionalInterpolationVelocity = interpolatedObject.positionalSmoothingFactor * elapsed_time;
			const float32 orientationalInterpolationVelocity =
			    interpolatedObject.orientationalSmoothingFactor * elapsed_time;
			finalPosition.X( InterpolateFloat32( transformComponentProxy.GetGlobalPosition( interpolatedTransform ).X(),
			                                     transformComponentProxy.GetGlobalPosition( ghostTransform ).X(),
			                                     positionalInterpolationVelocity ) );
			finalPosition.Y( InterpolateFloat32( transformComponentProxy.GetGlobalPosition( interpolatedTransform ).Y(),
			                                     transformComponentProxy.GetGlobalPosition( ghostTransform ).Y(),
			                                     positionalInterpolationVelocity ) );
			finalRotationAngle = MoveTowardsAngle( transformComponentProxy.GetGlobalRotation( interpolatedTransform ),
			                                       transformComponentProxy.GetGlobalRotation( ghostTransform ),
			                                       orientationalInterpolationVelocity );
		}

		newState.position = finalPosition;
		newState.rotationAngle = finalRotationAngle;
		interpolatedStateComponent.state = newState;
	}
}
