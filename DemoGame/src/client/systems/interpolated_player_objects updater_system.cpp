#include "interpolated_player_objects updater_system.h"

#include "ecs/world.h"
#include "ecs/game_entity.hpp"

#include "components/transform_component.h"

#include "client/components/ghost_object_component.h"
#include "client/components/interpolated_object_component.h"

#include "vec2f.h"

InterpolatedPlayerObjectUpdaterSystem::InterpolatedPlayerObjectUpdaterSystem()
    : Engine::ECS::ISimpleSystem()
{
}

static float32 GetMinimumDistanceBetweenAngles( float32 a, float32 b )
{
	const float32 delta = fmodf( b - a + 540.0f, 360.0f ) - 180.0f;
	return fabs( delta );
}

static bool IsSnapToGhostRequired( const Engine::TransformComponent& ghost,
                                   const Engine::TransformComponent& interpolated,
                                   const InterpolatedObjectComponent& interpolation_config )
{
	return Vec2f::GetSquareDistance( ghost.GetPosition(), interpolated.GetPosition() ) >=
	           interpolation_config.squaredSnapToGhostPositionDistanceThreshold ||
	       GetMinimumDistanceBetweenAngles( ghost.GetRotationAngle(), interpolated.GetRotationAngle() ) >=
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

void InterpolatedPlayerObjectUpdaterSystem::Execute( Engine::ECS::World& world, float32 elapsed_time )
{
	std::vector< Engine::ECS::GameEntity > interpolatedPlayerEntities =
	    world.GetEntitiesOfType< InterpolatedObjectComponent >();
	for ( auto it = interpolatedPlayerEntities.begin(); it != interpolatedPlayerEntities.end(); ++it )
	{
		const InterpolatedObjectComponent& interpolatedObject = it->GetComponent< InterpolatedObjectComponent >();
		const GhostObjectComponent& ghostObject = it->GetComponent< GhostObjectComponent >();

		if ( !ghostObject.entity.IsValid() )
		{
			LOG_WARNING( "The Ghost object associated with an interpolated object is not valid. Skipping it." );
			continue;
		}

		const Engine::TransformComponent& ghostTransform =
		    ghostObject.entity.GetComponent< Engine::TransformComponent >();
		Engine::TransformComponent& interpolatedTransform = it->GetComponent< Engine::TransformComponent >();

		Vec2f finalPosition;
		float32 finalRotationAngle;
		if ( IsSnapToGhostRequired( ghostTransform, interpolatedTransform, interpolatedObject ) )
		{
			finalPosition = ghostTransform.GetPosition();
			finalRotationAngle = ghostTransform.GetRotationAngle();
		}
		else
		{
			const float32 positionalInterpolationVelocity = interpolatedObject.positionalSmoothingFactor * elapsed_time;
			const float32 orientationalInterpolationVelocity =
			    interpolatedObject.orientationalSmoothingFactor * elapsed_time;
			finalPosition.X( InterpolateFloat32( interpolatedTransform.GetPosition().X(),
			                                     ghostTransform.GetPosition().X(), positionalInterpolationVelocity ) );
			finalPosition.Y( InterpolateFloat32( interpolatedTransform.GetPosition().Y(),
			                                     ghostTransform.GetPosition().Y(), positionalInterpolationVelocity ) );
			finalRotationAngle =
			    MoveTowardsAngle( interpolatedTransform.GetRotationAngle(), ghostTransform.GetRotationAngle(),
			                      orientationalInterpolationVelocity );
		}

		interpolatedTransform.SetPosition( finalPosition );
		interpolatedTransform.SetRotationAngle( finalRotationAngle );
	}
}
