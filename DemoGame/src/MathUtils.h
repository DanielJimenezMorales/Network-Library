#pragma once
#include "Vec2f.h"

float32 GetNoSquareRootDistance(const Vec2f& v1, const Vec2f& v2);
float32 GetDistance(const Vec2f& v1, const Vec2f& v2);
//We're taking the assumption here that angle 0 == (0, 1)
Vec2f ConvertAngleToNormalizedDirection(float32 angle);
