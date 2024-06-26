#pragma once
#include <SDL_image.h>

class Texture
{
public:
	Texture(SDL_Texture* texture, const SDL_Rect& dimensions) : _texture(texture), _texturePixelDimensions(dimensions)
	{
	}

	const SDL_Rect& GetDimensions() const { return _texturePixelDimensions; }
	SDL_Texture* GetRaw() const { return _texture; }

private:
	SDL_Texture* _texture;
	SDL_Rect _texturePixelDimensions;
};

class TextureLoader
{
public:
	TextureLoader() : _renderer(nullptr)
	{
	}

	void Init(SDL_Renderer* renderer);
	Texture* LoadTexture(const char* stringFilePath);

private:
	bool IsTextureCached(const char* stringFilePath) const;
	Texture* GetCachedTexture(const char* stringFilePath) const;
	void CacheTexture(const char* stringFilePath, Texture* texture);

	SDL_Renderer* _renderer;
};
