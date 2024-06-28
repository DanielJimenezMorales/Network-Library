#pragma once
#include "Vec2f.h"

struct TransformComponent
{
public:
	TransformComponent() : position(0.f, 0.f) {}
	TransformComponent(float x, float y) : position(x, y) {}

	Vec2f position;
};
