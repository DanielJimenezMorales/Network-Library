#pragma once
#include <SDL_image.h>

class EntityContainer;

class SpriteRendererSystem
{
public:
	void Render(EntityContainer& entityContainer, SDL_Renderer* renderer) const;
};
