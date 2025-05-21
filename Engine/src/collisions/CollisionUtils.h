#pragma once
#include "Vec2f.h"

struct TransformComponent;

bool CircleWithCircleOverlaps(const Vec2f& centerA, float32 radiusA, const Vec2f& centerB, float32 radiusB);
void SeparateCircleWithCircle(TransformComponent& transformA, float32 radiusA, TransformComponent& transformB, float32 radiusB);
