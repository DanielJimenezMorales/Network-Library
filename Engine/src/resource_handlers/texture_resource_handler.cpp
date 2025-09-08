#include "texture_resource_handler.h"

#include "numeric_types.h"
#include "logger.h"

#include "texture.h"

#include <SDL_image.h>
#include <cassert>

namespace Engine
{
	TextureResourceHandler::TextureResourceHandler( SDL_Renderer* renderer )
	    : _renderer( renderer )
	    , _stringFilePathToCachedTextureMap()
	{
		Allocate();
	}

	TextureResourceHandler::~TextureResourceHandler()
	{
		ClearCache();
	}

	TextureHandler TextureResourceHandler::LoadTexture( const char* stringFilePath )
	{
		if ( !IsTextureCached( stringFilePath ) )
		{
			if ( CreateTexture( stringFilePath ) == nullptr )
			{
				return TextureHandler::GetInvalid();
			}
		}

		return TextureHandler( stringFilePath );
	}

	const Texture* TextureResourceHandler::TryGetTextureFromHandler( const TextureHandler& handler ) const
	{
		assert( handler.IsValid() );

		return GetCachedTexture( handler.key.c_str() );
	}

	Texture* TextureResourceHandler::TryGetTextureFromHandler( const TextureHandler& handler )
	{
		return const_cast< Texture* >(
		    static_cast< const TextureResourceHandler& >( *this ).TryGetTextureFromHandler( handler ) );
	}

	bool TextureResourceHandler::IsTextureCached( const char* stringFilePath ) const
	{
		return GetCachedTexture( stringFilePath ) != nullptr;
	}

	Texture* TextureResourceHandler::GetCachedTexture( const char* stringFilePath ) const
	{
		Texture* texture = nullptr;
		auto textureFound = _stringFilePathToCachedTextureMap.find( stringFilePath );
		if ( textureFound != _stringFilePathToCachedTextureMap.end() )
		{
			texture = textureFound->second;
		}

		return texture;
	}

	Texture* TextureResourceHandler::CreateTexture( const char* stringFilePath )
	{
		SDL_Surface* imageSurface = IMG_Load( stringFilePath );
		if ( imageSurface == nullptr )
		{
			LOG_ERROR( "Can't load texture at %s. File not found", stringFilePath );
			return nullptr;
		}

		SDL_Texture* imageTexture = SDL_CreateTextureFromSurface( _renderer, imageSurface );
		if ( imageTexture == nullptr )
		{
			LOG_ERROR( "Can't load texture at %s. SDL error: %s", stringFilePath, SDL_GetError() );
			SDL_FreeSurface( imageSurface );
			return nullptr;
		}
		SDL_FreeSurface( imageSurface );

		int32 textureWidth = 0;
		int32 textureHeight = 0;
		int32 result = SDL_QueryTexture( imageTexture, NULL, NULL, &textureWidth, &textureHeight );
		if ( result != 0 )
		{
			LOG_ERROR( "Can't query texture at %s. Query error code: %d. SDL error: %s", stringFilePath, result,
			           SDL_GetError() );
			SDL_DestroyTexture( imageTexture );
			imageTexture = nullptr;
			return nullptr;
		}

		Texture* texture = new Texture( imageTexture, textureWidth, textureHeight );
		assert( texture != nullptr );

		CacheTexture( stringFilePath, texture );
		return texture;
	}

	void TextureResourceHandler::CacheTexture( const char* stringFilePath, Texture* texture )
	{
		assert( texture != nullptr );

		if ( IsTextureCached( stringFilePath ) )
		{
			LOG_WARNING( "Texture already cached. Texture path: %s", stringFilePath );
			return;
		}

		_stringFilePathToCachedTextureMap.emplace( stringFilePath, texture );
	}

	void TextureResourceHandler::Allocate()
	{
		_stringFilePathToCachedTextureMap.reserve( INITIAL_CACHE_POOL_SIZE );
	}

	void TextureResourceHandler::ClearCache()
	{
		auto it = _stringFilePathToCachedTextureMap.begin();
		for ( ; it != _stringFilePathToCachedTextureMap.end(); ++it )
		{
			delete it->second;
			it->second = nullptr;
		}

		_stringFilePathToCachedTextureMap.clear();
	}
} // namespace Engine
