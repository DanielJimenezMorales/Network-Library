#include "texture_asset_loader.h"
#include "asset_manager/asset.h"

#include "render/texture_asset.h"

#include "logger.h"

#include <SDL_image.h>
#include <cassert>

namespace Engine
{
	TextureAssetLoader::TextureAssetLoader( SDL_Renderer* renderer )
	    : _renderer( renderer )
	{
	}

	Asset* TextureAssetLoader::Load( const std::string& path )
	{
		SDL_Surface* imageSurface = IMG_Load( path.c_str() );
		if ( imageSurface == nullptr )
		{
			LOG_ERROR( "[%s] Can't load texture at %s. File not found", THIS_FUNCTION_NAME, path.c_str() );
			return nullptr;
		}

		SDL_Texture* imageTexture = SDL_CreateTextureFromSurface( _renderer, imageSurface );
		if ( imageTexture == nullptr )
		{
			LOG_ERROR( "[%s] Can't load texture at %s. SDL error: %s", THIS_FUNCTION_NAME, path.c_str(),
			           SDL_GetError() );
			SDL_FreeSurface( imageSurface );
			return nullptr;
		}
		SDL_FreeSurface( imageSurface );

		int32 textureWidth = 0;
		int32 textureHeight = 0;
		int32 result = SDL_QueryTexture( imageTexture, NULL, NULL, &textureWidth, &textureHeight );
		if ( result != 0 )
		{
			LOG_ERROR( "[%s] Can't query texture at %s. Query error code: %d. SDL error: %s", THIS_FUNCTION_NAME,
			           path.c_str(), result, SDL_GetError() );
			SDL_DestroyTexture( imageTexture );
			imageTexture = nullptr;
			return nullptr;
		}

		TextureAsset* texture = new TextureAsset( path, imageTexture, textureWidth, textureHeight );
		assert( texture != nullptr );

		return texture;
	}
} // namespace Engine
