#pragma once
#include "Vec2f.h"

struct TransformComponent;

bool CircleWithCircleOverlaps(const Vec2f& centerA, float radiusA, const Vec2f& centerB, float radiusB);
void SeparateCircleWithCircle(TransformComponent& transformA, float radiusA, TransformComponent& transformB, float radiusB);
