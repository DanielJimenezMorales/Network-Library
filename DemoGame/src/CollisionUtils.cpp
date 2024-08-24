#include "CollisionUtils.h"
#include "MathUtils.h"
#include "TransformComponent.h"
#include <cassert>

bool CircleWithCircleOverlaps(const Vec2f& centerA, float radiusA, const Vec2f& centerB, float radiusB)
{
	const float squareRadiusSum = (radiusA * radiusA) + (radiusB * radiusB);
	const float distanceBetweenCenters = GetNoSquareRootDistance(centerA, centerB);
	return distanceBetweenCenters < squareRadiusSum;
}

void SeparateCircleWithCircle(TransformComponent& transformA, float radiusA, TransformComponent& transformB, float radiusB)
{

	const float radiusSum = radiusA + radiusB;
	const float distanceBetweenCenters = GetDistance(transformA.GetPosition(), transformB.GetPosition());
	const float halfDistanceDifference = (radiusSum - distanceBetweenCenters) / 2.f;
	assert(halfDistanceDifference >= 0.f);

	TransformComponent& leftCircle = (transformA.GetPosition().X() <= transformB.GetPosition().X()) ? transformA : transformB;
	TransformComponent& rightCircle = (transformA.GetPosition().X() <= transformB.GetPosition().X()) ? transformB : transformA;

	Vec2f leftToRightDirection = rightCircle.GetPosition() - leftCircle.GetPosition();
	leftToRightDirection.Normalize();

	rightCircle.SetPosition(rightCircle.GetPosition() + (leftToRightDirection * halfDistanceDifference));
	leftCircle.SetPosition(leftCircle.GetPosition() - (leftToRightDirection * halfDistanceDifference));
}
