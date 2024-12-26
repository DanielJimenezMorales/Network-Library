#pragma once
#include "numeric_types.h"

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
	Gizmo(uint8 r, uint8 g, uint8 b, uint8 a) : r(r), g(g), b(b), a(a)
	{
	}

	virtual void RenderConcrete(const CameraComponent& cameraComponent, SDL_Renderer* renderer) const = 0;

private:
	const uint8 r, g, b, a;
};
