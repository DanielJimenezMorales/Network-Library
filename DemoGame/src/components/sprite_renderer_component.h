#pragma once
#include "resource_handlers/texture_resource_handler.h"

struct SpriteRendererComponent
{
	public:
		SpriteRendererComponent( const TextureHandler& texture_handler )
		    : textureHandler( texture_handler )
		{
		}

		TextureHandler textureHandler;
};
