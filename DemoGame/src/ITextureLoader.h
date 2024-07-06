#pragma once

class Texture;

class ITextureLoader
{
public:
	virtual Texture* LoadTexture(const char* stringFilePath) = 0;
};