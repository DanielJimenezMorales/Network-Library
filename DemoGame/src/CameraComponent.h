#pragma once
#include "Vec2f.h"

struct CameraComponent
{
	static constexpr uint32_t PIXELS_PER_WORLD_UNIT = 10;

	CameraComponent(const Vec2f& position, int width, int height) : position(position), width(width), height(height)
	{
	}

	Vec2f ConvertFromWorldPositionToScreenPosition(Vec2f worldPosition) const
	{
		//Conver from world coordinates to uncentered screen coordinates
		worldPosition = (worldPosition - position) * PIXELS_PER_WORLD_UNIT;

		//Center position based on windows size (SDL2's center of coordinates is the top left position of the screen)
		worldPosition.AddToX(static_cast<float>(width / 2));
		worldPosition.AddToY(static_cast<float>(height / 2));
		return worldPosition;
	}

	Vec2f ConvertFromScreenPositionToWorldPosition(Vec2f screenPosition) const
	{
		screenPosition.AddToX(-static_cast<float>(width / 2));
		screenPosition.AddToY(-static_cast<float>(height / 2));

		screenPosition /= PIXELS_PER_WORLD_UNIT;
		screenPosition.AddToX(position.X());
		screenPosition.AddToY(position.Y());
		return screenPosition;
	}

	Vec2f position;
	int width;
	int height;
};