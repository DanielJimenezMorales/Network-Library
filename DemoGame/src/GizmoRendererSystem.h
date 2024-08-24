#pragma once
#include "SDL.h"

class EntityContainer;

class GizmoRendererSystem
{
public:
	void Render(EntityContainer& entityContainer, SDL_Renderer* renderer) const;
};
