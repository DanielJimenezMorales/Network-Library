#include "sprite_renderer_system.h"

#include "coordinates_conversion_utils.h"
#include "Texture.h"
#include "Vec2f.h"
#include "Logger.h"

#include "ecs/game_entity.hpp"
#include "ecs/world.h"
#include "ecs/prefab.h"

#include "global_components/render_global_component.h"

#include "components/transform_component.h"
#include "components/sprite_renderer_component.h"
#include "components/camera_component.h"

#include "component_configurations/sprite_renderer_component_configuration.h"

#include <cassert>

namespace Engine
{
	SpriteRendererSystem::SpriteRendererSystem( SDL_Renderer* renderer )
	    : _textureResourceHandler( renderer )
	{
	}

	void SpriteRendererSystem::Execute( ECS::World& world, float32 elapsed_time )
	{
		RenderGlobalComponent& render_global_component = world.GetGlobalComponent< RenderGlobalComponent >();

		const ECS::GameEntity camera_entity = world.GetFirstEntityOfType< CameraComponent >();
		const CameraComponent& camera = camera_entity.GetComponent< CameraComponent >();
		const TransformComponent& camera_transform = camera_entity.GetComponent< TransformComponent >();

		std::vector< ECS::GameEntity > entities =
		    world.GetEntitiesOfBothTypes< SpriteRendererComponent, TransformComponent >();
		for ( auto it = entities.begin(); it != entities.end(); ++it )
		{
			// auto [spriteRenderer, transform] = view.get<SpriteRendererComponent, TransformComponent>(entity);
			const SpriteRendererComponent& spriteRenderer = it->GetComponent< SpriteRendererComponent >();
			const TransformComponent& transform = it->GetComponent< TransformComponent >();

			const Vec2f screenPosition =
			    ConvertFromWorldPositionToScreenPosition( transform.GetPosition(), camera, camera_transform );
			const Texture* texture = _textureResourceHandler.TryGetTextureFromHandler( spriteRenderer.textureHandler );
			if ( texture == nullptr )
			{
				continue;
			}

			SDL_Rect destRect;
			destRect.x = static_cast< int >( screenPosition.X() - ( texture->GetDimensions().w / 2.f ) );
			destRect.y = static_cast< int >( screenPosition.Y() - ( texture->GetDimensions().h / 2.f ) );
			destRect.w = static_cast< int >( texture->GetDimensions().w * transform.GetScale().X() );
			destRect.h = static_cast< int >( texture->GetDimensions().h * transform.GetScale().Y() );

			// SDL_RenderCopy(renderer, texture->GetRaw(), &texture->GetDimensions(), &destRect);
			SDL_RenderCopyEx( render_global_component.renderer, texture->GetRaw(), &texture->GetDimensions(), &destRect,
			                  transform.GetRotationAngle(), nullptr, SDL_RendererFlip::SDL_FLIP_NONE );
		}
	}

	void SpriteRendererSystem::ConfigureSpriteRendererComponent( ECS::GameEntity& entity, const ECS::Prefab& prefab )
	{
		auto component_config_found = prefab.componentConfigurations.find( "SpriteRenderer" );
		if ( component_config_found == prefab.componentConfigurations.end() )
		{
			return;
		}

		if ( !entity.HasComponent< SpriteRendererComponent >() )
		{
			return;
		}

		const SpriteRendererComponentConfiguration& sprite_renderer_config =
		    static_cast< const SpriteRendererComponentConfiguration& >( *component_config_found->second );
		SpriteRendererComponent& sprite_renderer = entity.GetComponent< SpriteRendererComponent >();
		sprite_renderer.textureHandler =
		    _textureResourceHandler.LoadTexture( sprite_renderer_config.texturePath.c_str() );
	}
} // namespace Engine
