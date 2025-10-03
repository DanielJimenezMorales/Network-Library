#pragma once
#include "vec2f.h"

#include "resource_handlers/texture_resource_handler.h"

namespace Engine
{
	enum class SpriteType
	{
		SINGLE = 0,
		SPRITE_SHEET = 1
	};

	struct SpriteRendererComponent
	{
		public:
			SpriteRendererComponent()
			    : textureHandler( TextureHandler::GetInvalid() )
			    , type( SpriteType::SINGLE )
			    , width( 0 )
			    , height( 0 )
			    , uv0( 0.f, 0.f )
			    , uv1( 1.f, 1.f )
			    , flipX( false )
			    , flipY( false )
			{
			}

			SpriteRendererComponent( const TextureHandler& texture_handler )
			    : textureHandler( texture_handler )
			    , type( SpriteType::SINGLE )
			    , width( 0 )
			    , height( 0 )
			    , uv0( 0.f, 0.f )
			    , uv1( 1.f, 1.f )
			    , flipX( false )
			    , flipY( false )
			{
			}

			TextureHandler textureHandler;
			SpriteType type;
			uint32 width;
			uint32 height;
			Vec2f uv0;
			Vec2f uv1;
			bool flipX;
			bool flipY;
	};
} // namespace Engine
