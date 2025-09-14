#include "collision_utils.h"

#include "math_utils.h"

#include "transform_component_proxy.h"

#include <cassert>

namespace Engine
{
	bool CircleWithCircleOverlaps( const Vec2f& centerA, float32 radiusA, const Vec2f& centerB, float32 radiusB )
	{
		const float32 squareRadiusSum = ( radiusA * radiusA ) + ( radiusB * radiusB );
		const float32 distanceBetweenCenters = GetNoSquareRootDistance( centerA, centerB );
		return distanceBetweenCenters < squareRadiusSum;
	}

	void SeparateCircleWithCircle( TransformComponentProxy& transformA, float32 radiusA,
	                               TransformComponentProxy& transformB, float32 radiusB )
	{
		const float32 radiusSum = radiusA + radiusB;
		const float32 distanceBetweenCenters =
		    GetDistance( transformA.GetGlobalPosition(), transformB.GetGlobalPosition() );
		const float32 halfDistanceDifference = ( radiusSum - distanceBetweenCenters ) / 2.f;
		assert( halfDistanceDifference >= 0.f );

		TransformComponentProxy& leftCircle =
		    ( transformA.GetGlobalPosition().X() <= transformB.GetGlobalPosition().X() ) ? transformA : transformB;
		TransformComponentProxy& rightCircle =
		    ( transformA.GetGlobalPosition().X() <= transformB.GetGlobalPosition().X() ) ? transformB : transformA;

		Vec2f leftToRightDirection = rightCircle.GetGlobalPosition() - leftCircle.GetGlobalPosition();
		leftToRightDirection.Normalize();

		rightCircle.SetGlobalPosition( rightCircle.GetGlobalPosition() +
		                               ( leftToRightDirection * halfDistanceDifference ) );
		leftCircle.SetGlobalPosition( leftCircle.GetGlobalPosition() -
		                              ( leftToRightDirection * halfDistanceDifference ) );
	}
}
