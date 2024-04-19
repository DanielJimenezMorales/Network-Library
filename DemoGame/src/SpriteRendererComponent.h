#pragma once
#include <SDL_image.h>

struct SpriteRendererComponent
{
public:
	SpriteRendererComponent(const SDL_Rect& srcRect, SDL_Texture* tex) : sourceRect(srcRect), texture(tex) {};

	SDL_Rect sourceRect;
	SDL_Texture* texture;
};
