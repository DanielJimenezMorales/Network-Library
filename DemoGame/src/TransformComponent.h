#pragma once
#include "Vec2f.h"

struct TransformComponent
{
public:
	TransformComponent() : position(0.f, 0.f), lookAtDirection(0.f, -1.f), previousLookAtDirection(0.f, -1.f) {}
	TransformComponent(float x, float y) : position(x, y), lookAtDirection(0.f, -1.f), previousLookAtDirection(0.f, -1.f) {}

	void LookAt(const Vec2f& position)
	{
		Vec2f direction = position - this->position;
		direction.Normalize();
		lookAtDirection = direction;
	}

	Vec2f position;
	Vec2f lookAtDirection;
	Vec2f previousLookAtDirection;
};
