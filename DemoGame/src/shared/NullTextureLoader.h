#pragma once
#include "ITextureLoader.h"
#include "logger.h"

class NullTextureLoader : public ITextureLoader
{
public:
	Texture* LoadTexture(const char* stringFilePath) override
	{
		LOG_WARNING("NullTextureLoader calling to LoadTexture");
		return nullptr;
	}
};