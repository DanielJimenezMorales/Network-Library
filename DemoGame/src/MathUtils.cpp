#include "MathUtils.h"
#include <cmath>

float GetNoSquareRootDistance(const Vec2f& v1, const Vec2f& v2)
{
	const float dx = v2.X() - v1.X();
	const float dy = v2.Y() - v1.Y();
	return (dx * dx) + (dy * dy);
}

float GetDistance(const Vec2f& v1, const Vec2f& v2)
{
	const float dx = v2.X() - v1.X();
	const float dy = v2.Y() - v1.Y();
	return sqrtf((dx * dx) + (dy * dy));
}
