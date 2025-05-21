#pragma once
#include "resource_handlers/texture_resource_handler.h"

namespace Engine
{
	struct SpriteRendererComponent
	{
		public:
			SpriteRendererComponent()
			    : textureHandler( TextureHandler::GetInvalid() )
			{
			}

			SpriteRendererComponent( const TextureHandler& texture_handler )
			    : textureHandler( texture_handler )
			{
			}

			TextureHandler textureHandler;
	};
}
