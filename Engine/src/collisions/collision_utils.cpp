#include "collision_utils.h"

#include "math_utils.h"

#include "components/transform_component.h"

#include "transform/transform_hierarchy_helper_functions.h"

#include <cassert>

namespace Engine
{
	bool CircleWithCircleOverlaps( const Vec2f& centerA, float32 radiusA, const Vec2f& centerB, float32 radiusB )
	{
		const float32 squareRadiusSum = ( radiusA * radiusA ) + ( radiusB * radiusB );
		const float32 distanceBetweenCenters = GetNoSquareRootDistance( centerA, centerB );
		return distanceBetweenCenters < squareRadiusSum;
	}

	void SeparateCircleWithCircle( TransformComponent& transformA, float32 radiusA, TransformComponent& transformB,
	                               float32 radiusB )
	{
		const TransformComponentProxy transformComponentProxy;

		const float32 radiusSum = radiusA + radiusB;
		const float32 distanceBetweenCenters = GetDistance( transformComponentProxy.GetGlobalPosition( transformA ),
		                                                    transformComponentProxy.GetGlobalPosition( transformB ) );
		const float32 halfDistanceDifference = ( radiusSum - distanceBetweenCenters ) / 2.f;
		assert( halfDistanceDifference >= 0.f );

		TransformComponent& leftCircle = ( transformComponentProxy.GetGlobalPosition( transformA ).X() <=
		                                   transformComponentProxy.GetGlobalPosition( transformB ).X() )
		                                     ? transformA
		                                     : transformB;
		TransformComponent& rightCircle = ( transformComponentProxy.GetGlobalPosition( transformA ).X() <=
		                                    transformComponentProxy.GetGlobalPosition( transformB ).X() )
		                                      ? transformB
		                                      : transformA;

		Vec2f leftToRightDirection = transformComponentProxy.GetGlobalPosition( rightCircle ) -
		                             transformComponentProxy.GetGlobalPosition( leftCircle );
		leftToRightDirection.Normalize();

		transformComponentProxy.SetGlobalPosition( rightCircle,
		                                           transformComponentProxy.GetGlobalPosition( rightCircle ) +
		                                               ( leftToRightDirection * halfDistanceDifference ) );
		transformComponentProxy.SetGlobalPosition( leftCircle, transformComponentProxy.GetGlobalPosition( leftCircle ) -
		                                                           ( leftToRightDirection * halfDistanceDifference ) );
	}
}
