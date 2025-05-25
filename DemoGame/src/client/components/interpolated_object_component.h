#pragma once

#include "numeric_types.h"

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

		float32 positionalSmoothingFactor;
		float32 orientationalSmoothingFactor;
		float32 snapToGhostPositionDistanceThreshold;
		const float32 squaredSnapToGhostPositionDistanceThreshold;
		float32 snapToGhostOrientationThreshold;
};