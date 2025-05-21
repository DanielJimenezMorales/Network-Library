#pragma once
#include "numeric_types.h"

#include <unordered_map>
#include <string>

struct SDL_Renderer;
class Texture;

static constexpr uint32 INITIAL_CACHE_POOL_SIZE = 1024;

struct TextureHandler
{
		TextureHandler( const char* key )
		    : key( key )
		{
		}

		static TextureHandler GetInvalid() { return TextureHandler( "\0" ); }

		bool IsValid() const { return strcmp( key.c_str(), "\0" ) != 0; }

		std::string key;
};

/// <summary>
/// This class is a centralized and safe storage for texture resources. It contains an internal cache system in order to
/// reduce texture loading the same texture multiple times.
/// </summary>
class TextureResourceHandler
{
	public:
		TextureResourceHandler( SDL_Renderer* renderer );

		~TextureResourceHandler();

		/// <summary>
		/// Loads a texture and returns an associated handler. If the texture is loaded for the very first time, it'll
		/// be loaded saved in a cache, otherwise it'll be reused.
		/// </summary>
		TextureHandler LoadTexture( const char* stringFilePath );

		/// <summary>
		/// <para>Gets the raw resource (texture) from a handler.</para>
		/// <para>IMPORTANT:</para>
		/// <para> 1. Do not save the returned pointer under any circumstance.</para>
		/// <para> 2. Do not pass the returned pointer as a parameter to other functions.</para>
		/// </summary>
		const Texture* TryGetTextureFromHandler( const TextureHandler& handler ) const;
		/// <summary>
		/// <para>Gets the raw resource (texture) from a handler.</para>
		/// <para>IMPORTANT:</para>
		/// <para> 1. Do not save the returned pointer under any circumstance.</para>
		/// <para> 2. Do not pass the returned pointer as a parameter to other functions.</para>
		/// </summary>
		Texture* TryGetTextureFromHandler( const TextureHandler& handler );

	private:
		bool IsTextureCached( const char* stringFilePath ) const;
		Texture* GetCachedTexture( const char* stringFilePath ) const;
		Texture* CreateTexture( const char* stringFilePath );
		void CacheTexture( const char* stringFilePath, Texture* texture );

		void Allocate();
		void ClearCache();

		SDL_Renderer* _renderer;
		std::unordered_map< std::string, Texture* > _stringFilePathToCachedTextureMap;
};
