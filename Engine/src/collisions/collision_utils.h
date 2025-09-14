#pragma once
#include "vec2f.h"

namespace Engine
{
	class TransformComponentProxy;

	bool CircleWithCircleOverlaps( const Vec2f& centerA, float32 radiusA, const Vec2f& centerB, float32 radiusB );
	void SeparateCircleWithCircle( TransformComponentProxy& transformA, float32 radiusA,
	                               TransformComponentProxy& transformB, float32 radiusB );
}
