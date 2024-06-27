#pragma once
#include <unordered_map>
#include <string>

class Texture;
struct SDL_Renderer;

class TextureLoader
{
public:
	TextureLoader() : _renderer(nullptr), _stringFilePathToCachedTextureMap()
	{
	}

	~TextureLoader();

	void Init(SDL_Renderer* renderer);
	Texture* LoadTexture(const char* stringFilePath);

private:
	bool IsTextureCached(const char* stringFilePath) const;
	Texture* GetCachedTexture(const char* stringFilePath) const;
	Texture* CreateTexture(const char* stringFilePath);
	void CacheTexture(const char* stringFilePath, Texture* texture);

	void ClearCache();

	SDL_Renderer* _renderer;
	std::unordered_map<std::string, Texture*> _stringFilePathToCachedTextureMap;
};
