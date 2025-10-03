#pragma once
#include "numeric_types.h"

#include "ecs/game_entity.hpp"

/// <summary>
/// This component is used to mark objects that should have their position and orientation interpolated in order to
/// mimic another entity's position and orientation (called Ghost Entity)
/// </summary>
struct InterpolatedObjectComponent
{
		InterpolatedObjectComponent()
		    : positionalSmoothingFactor( 25.f )
		    , orientationalSmoothingFactor(
		          800.f ) // rotational is higher because the mouse movement can be crazy sometimes.
		    , snapToGhostPositionDistanceThreshold( 1.0f )
		    , squaredSnapToGhostPositionDistanceThreshold( snapToGhostPositionDistanceThreshold *
		                                                   snapToGhostPositionDistanceThreshold )
		    , snapToGhostOrientationThreshold( 1.0f )
		{
		}

		// The entity to interpolate towards
		Engine::ECS::GameEntity ghostEntity;

		// If invalid, rotation will be applied to the entity with this component, otherwise, it will be apply to
		// interpolatedRotationChildEntity.
		// IMPORTANT: interpolatedRotationChildEntity must be a child entity.
		Engine::ECS::GameEntity interpolatedRotationChildEntity;

		float32 positionalSmoothingFactor;
		float32 orientationalSmoothingFactor;
		float32 snapToGhostPositionDistanceThreshold;
		const float32 squaredSnapToGhostPositionDistanceThreshold;
		float32 snapToGhostOrientationThreshold;
};