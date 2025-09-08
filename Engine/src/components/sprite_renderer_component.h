#pragma once
#include "vec2f.h"

#include "resource_handlers/texture_resource_handler.h"

namespace Engine
{
	struct SpriteRendererComponent
	{
		public:
			SpriteRendererComponent()
			    : textureHandler( TextureHandler::GetInvalid() )
			    , uv0( 0.f, 0.f )
			    , uv1( 1.f, 1.f )
			{
			}

			SpriteRendererComponent( const TextureHandler& texture_handler )
			    : textureHandler( texture_handler )
			    , uv0( 0.f, 0.f )
			    , uv1( 1.f, 1.f )
			{
			}

			TextureHandler textureHandler;
			Vec2f uv0;
			Vec2f uv1;
	};
}
