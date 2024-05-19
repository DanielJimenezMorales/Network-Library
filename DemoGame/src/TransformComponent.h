#pragma once

struct TransformComponent
{
public:
	TransformComponent() : posX(0.f), posY(0.f) {};
	TransformComponent(float x, float y) : posX(x), posY(y) {};

	float posX;
	float posY;
};
