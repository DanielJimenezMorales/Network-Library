#include "TextureLoader.h"
#include "Logger.h"
#include <cassert>

void TextureLoader::Init(SDL_Renderer* renderer)
{
	_renderer = renderer;
}

Texture* TextureLoader::LoadTexture(const char* stringFilePath)
{
	Texture* texture = nullptr;
	if (IsTextureCached(stringFilePath))
	{
		texture = GetCachedTexture(stringFilePath);
		assert(texture != nullptr);

		return texture;
	}

	SDL_Surface* imageSurface = IMG_Load(stringFilePath);
	if (imageSurface == nullptr)
	{
		LOG_ERROR("Can't load texture at %s. File not found", stringFilePath);
		return nullptr;
	}

	SDL_Texture* imageTexture = SDL_CreateTextureFromSurface(_renderer, imageSurface);
	if (imageTexture == nullptr)
	{
		LOG_ERROR("Can't load texture at %s. SDL error: %s", stringFilePath, SDL_GetError());
		SDL_FreeSurface(imageSurface);
		return nullptr;
	}
	SDL_FreeSurface(imageSurface);

	SDL_Rect sourceTextureRect;
	int result = SDL_QueryTexture(imageTexture, NULL, NULL, &sourceTextureRect.w, &sourceTextureRect.h);
	if (result != 0)
	{
		LOG_ERROR("Can't query texture at %s. Query error code: %d. SDL error: %s", stringFilePath, result, SDL_GetError());
		SDL_DestroyTexture(imageTexture);
		imageTexture = nullptr;
		return nullptr;
	}

	sourceTextureRect.x = 0;
	sourceTextureRect.y = 0;

	texture = new Texture(imageTexture, sourceTextureRect);
	assert(texture != nullptr);

	return texture;
}

bool TextureLoader::IsTextureCached(const char* stringFilePath) const
{
	return false;
}

Texture* TextureLoader::GetCachedTexture(const char* stringFilePath) const
{
	return nullptr;
}

void TextureLoader::CacheTexture(const char* stringFilePath, Texture* texture)
{
	assert(!IsTextureCached(stringFilePath));


}
