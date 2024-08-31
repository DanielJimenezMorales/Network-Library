#include "MathUtils.h"
#include <cmath>

float32 GetNoSquareRootDistance(const Vec2f& v1, const Vec2f& v2)
{
	const float32 dx = v2.X() - v1.X();
	const float32 dy = v2.Y() - v1.Y();
	return (dx * dx) + (dy * dy);
}

float32 GetDistance(const Vec2f& v1, const Vec2f& v2)
{
	const float32 dx = v2.X() - v1.X();
	const float32 dy = v2.Y() - v1.Y();
	return sqrtf((dx * dx) + (dy * dy));
}
