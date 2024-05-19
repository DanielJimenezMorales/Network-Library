#pragma once
#include <SDL_image.h>
#include "entt.hpp"

class SpriteRendererSystem
{
public:
	void Render(const entt::registry& registry, SDL_Renderer* renderer) const;
};
