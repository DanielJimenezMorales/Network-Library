#pragma once
#include "ecs/i_simple_system.h"

#include "resource_handlers/texture_resource_handler.h"

#include "SDL_image.h"

namespace Engine
{
	namespace ECS
	{
		class Prefab;
	}

	class SpriteRendererSystem : public ECS::ISimpleSystem
	{
		public:
			SpriteRendererSystem( SDL_Renderer* renderer );

			void Execute( ECS::World& world, float32 elapsed_time ) override;

			TextureResourceHandler* GetTextureResourceHandler() { return &_textureResourceHandler; }

			void ConfigureSpriteRendererComponent( ECS::GameEntity& entity, const ECS::Prefab& prefab );

		private:
			TextureResourceHandler _textureResourceHandler;
	};
} // namespace Engine
