#include "sprite_renderer_system.h"

#include "coordinates_conversion_utils.h"
#include "vec2f.h"
#include "logger.h"

#include "ecs/game_entity.hpp"
#include "ecs/world.h"
#include "ecs/prefab.h"

#include "global_components/render_global_component.h"

#include "components/sprite_renderer_component.h"
#include "components/camera_component.h"
#include "components/transform_component.h"

#include "transform/transform_hierarchy_helper_functions.h"

#include "component_configurations/sprite_renderer_component_configuration.h"

#include "asset_manager/asset_manager.h"

#include "render/texture_asset.h"

#include <cassert>

namespace Engine
{
	SpriteRendererSystem::SpriteRendererSystem( SDL_Renderer* renderer, AssetManager* asset_manager )
	    : _assetManager( asset_manager )
	{
	}

	void SpriteRendererSystem::Execute( ECS::World& world, float32 elapsed_time )
	{
		RenderGlobalComponent& render_global_component = world.GetGlobalComponent< RenderGlobalComponent >();

		const ECS::GameEntity camera_entity = world.GetFirstEntityOfType< CameraComponent >();
		const CameraComponent& camera = camera_entity.GetComponent< CameraComponent >();
		const TransformComponent& cameraTransform = camera_entity.GetComponent< TransformComponent >();
		const TransformComponentProxy transformComponentProxy;

		std::vector< ECS::GameEntity > entities =
		    world.GetEntitiesOfBothTypes< SpriteRendererComponent, TransformComponent >();
		for ( auto it = entities.begin(); it != entities.end(); ++it )
		{
			// auto [spriteRenderer, transform] = view.get<SpriteRendererComponent, TransformComponent>(entity);
			const SpriteRendererComponent& spriteRenderer = it->GetComponent< SpriteRendererComponent >();
			if ( !spriteRenderer.textureHandler.IsValid() || spriteRenderer.isDisabled )
			{
				continue;
			}

			const TextureAsset* texture =
			    _assetManager->GetRawAsset< TextureAsset >( spriteRenderer.textureHandler, AssetType::TEXTURE );
			assert( texture != nullptr );

			SDL_Rect srcRect;
			srcRect.x = static_cast< int32 >( spriteRenderer.uv0.X() * texture->GetWidth() );
			srcRect.y = static_cast< int32 >( spriteRenderer.uv0.Y() * texture->GetHeight() );
			srcRect.w =
			    static_cast< int32 >( ( spriteRenderer.uv1.X() - spriteRenderer.uv0.X() ) * texture->GetWidth() );
			srcRect.h =
			    static_cast< int32 >( ( spriteRenderer.uv1.Y() - spriteRenderer.uv0.Y() ) * texture->GetHeight() );

			TransformComponent& transform = it->GetComponent< TransformComponent >();
			const Vec2f screenPosition = ConvertFromWorldPositionToScreenPosition(
			    transformComponentProxy.GetGlobalPosition( transform ), camera, cameraTransform );

			SDL_Rect destRect;
			destRect.x = static_cast< int >( screenPosition.X() - ( texture->GetWidth() / 2.f ) );
			destRect.y = static_cast< int >( screenPosition.Y() - ( texture->GetHeight() / 2.f ) );
			destRect.w =
			    static_cast< int >( texture->GetWidth() * transformComponentProxy.GetGlobalScale( transform ).X() );
			destRect.h =
			    static_cast< int >( texture->GetHeight() * transformComponentProxy.GetGlobalScale( transform ).Y() );

			SDL_Point center;
			center.x = texture->GetWidth() / 2;
			center.y = texture->GetHeight() / 2;

			// TODO So far, this doesn't support FLIP X and Y at the same time. If both are set, FLIP Y will only be
			// applied
			SDL_RendererFlip flip = SDL_RendererFlip::SDL_FLIP_NONE;
			if ( spriteRenderer.flipX )
			{
				flip = SDL_RendererFlip::SDL_FLIP_HORIZONTAL;
			}
			else if ( spriteRenderer.flipY )
			{
				flip = SDL_RendererFlip::SDL_FLIP_VERTICAL;
			}

			// SDL Rotates clockwise (the opposite as the engine that does it anti-clockwise), so we need to invert it.
			const float64 rotationAngle = 360 - transformComponentProxy.GetGlobalRotation( transform );
			SDL_RenderCopyEx( render_global_component.renderer, texture->GetRaw(), &srcRect, &destRect, rotationAngle,
			                  &center, flip );
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
		    _assetManager->GetAsset( sprite_renderer_config.texturePath, AssetType::TEXTURE );
		const TextureAsset* texture =
		    _assetManager->GetRawAsset< TextureAsset >( sprite_renderer.textureHandler, AssetType::TEXTURE );
		assert( texture != nullptr );
		sprite_renderer.type = sprite_renderer_config.type;
		sprite_renderer.width = texture->GetWidth();
		sprite_renderer.height = texture->GetHeight();
	}
} // namespace Engine
