#pragma once
#include "Gizmo.h"
#include "Vec2f.h"

class CircleGizmo : public Gizmo
{
public:
	CircleGizmo(const Vec2f& position, float radius) : Gizmo(0, 255, 0, 255), _position(position), _radius(radius)
	{
	}

protected:
	void RenderConcrete(const CameraComponent& cameraComponent, SDL_Renderer* renderer) const override;

private:
	const Vec2f _position;
	const float _radius;
};
