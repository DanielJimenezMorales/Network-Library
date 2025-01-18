#pragma once
#include "ecs/i_simple_system.h"

#include "resource_handlers/texture_resource_handler.h"

#include <SDL_image.h>

class SpriteRendererSystem : public ECS::ISimpleSystem
{
	public:
		SpriteRendererSystem( SDL_Renderer* renderer );

		void Execute( ECS::EntityContainer& entity_container, float32 elapsed_time ) override;

		TextureResourceHandler* GetTextureResourceHandler() { return &_textureResourceHandler; }

	private:
		SDL_Renderer* _renderer;
		TextureResourceHandler _textureResourceHandler;
};
