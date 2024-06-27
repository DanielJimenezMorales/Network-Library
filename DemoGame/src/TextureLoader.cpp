#include "TextureLoader.h"
#include "Logger.h"
#include "Texture.h"
#include <SDL_image.h>
#include <cassert>

TextureLoader::~TextureLoader()
{
	ClearCache();
}

void TextureLoader::Init(SDL_Renderer* renderer)
{
	_renderer = renderer;
}

Texture* TextureLoader::LoadTexture(const char* stringFilePath)
{
	if (IsTextureCached(stringFilePath))
	{
		Texture* texture = GetCachedTexture(stringFilePath);
		assert(texture != nullptr);

		return texture;
	}

	return CreateTexture(stringFilePath);
}

bool TextureLoader::IsTextureCached(const char* stringFilePath) const
{
	return GetCachedTexture(stringFilePath) != nullptr;
}

Texture* TextureLoader::GetCachedTexture(const char* stringFilePath) const
{
	Texture* texture = nullptr;
	auto textureFound = _stringFilePathToCachedTextureMap.find(stringFilePath);
	if (textureFound != _stringFilePathToCachedTextureMap.end())
	{
		texture = textureFound->second;
	}

	return texture;
}

Texture* TextureLoader::CreateTexture(const char* stringFilePath)
{
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

	Texture* texture = new Texture(imageTexture, sourceTextureRect);
	assert(texture != nullptr);

	CacheTexture(stringFilePath, texture);
	return texture;
}

void TextureLoader::CacheTexture(const char* stringFilePath, Texture* texture)
{
	assert(texture != nullptr);

	if (IsTextureCached(stringFilePath))
	{
		LOG_WARNING("Texture already cached. Texture path: %s", stringFilePath);
		return;
	}

	_stringFilePathToCachedTextureMap.emplace(stringFilePath, texture);
}

void TextureLoader::ClearCache()
{
	auto it = _stringFilePathToCachedTextureMap.begin();
	for (; it != _stringFilePathToCachedTextureMap.end(); ++it)
	{
		delete it->second;
		it->second = nullptr;
	}

	_stringFilePathToCachedTextureMap.clear();
}
