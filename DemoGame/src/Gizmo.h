#pragma once
#include <cstdint>
#include "SDL.h"

struct CameraComponent;

class Gizmo
{
public:
	virtual ~Gizmo() {}

	void Render(const CameraComponent& cameraComponent, SDL_Renderer* renderer) const
	{
		SDL_SetRenderDrawColor(renderer, r, g, b, a);
		RenderConcrete(cameraComponent, renderer);
	}

protected:
	Gizmo(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a)
	{
	}

	virtual void RenderConcrete(const CameraComponent& cameraComponent, SDL_Renderer* renderer) const = 0;

private:
	const uint8_t r, g, b, a;
};
