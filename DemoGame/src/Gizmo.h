#pragma once
#include "Vec2f.h"
#include <cstdint>
#include "SDL.h"

struct CameraComponent;

class Gizmo
{
public:
	Gizmo(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a)
	{
	}

	virtual ~Gizmo() {}

	virtual void Render(const CameraComponent& cameraComponent, SDL_Renderer* renderer) const
	{
		SDL_SetRenderDrawColor(renderer, r, g, b, a);
	}

private:
	const uint8_t r, g, b, a;
};

class CircleGizmo : public Gizmo
{
public:
	CircleGizmo(const Vec2f& position, float radius) : Gizmo(0, 255, 0, 255), _position(position), _radius(radius)
	{
	}

	void Render(const CameraComponent& cameraComponent, SDL_Renderer* renderer) const override;

private:
	const Vec2f _position;
	const float _radius;
};
