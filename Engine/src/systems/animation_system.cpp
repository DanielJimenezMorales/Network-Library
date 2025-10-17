#include "animation_system.h"

#include "logger.h"

#include "ecs/world.h"
#include "ecs/game_entity.hpp"
#include "ecs/prefab.h"

#include "components/animation_component.h"
#include "components/sprite_renderer_component.h"

#include "component_configurations/animation_component_configuration.h"

#include "asset_manager/asset_manager.h"
#include "asset_manager/asset_handle.h"

#include "animation/animation_asset.h"
#include "animation/animation_component_proxy.h"

#include "render/texture_asset.h"

namespace Engine
{
	AnimationSystem::AnimationSystem( AssetManager* asset_manager )
	    : ISimpleSystem()
	    , _assetManager( asset_manager )
	{
		assert( _assetManager != nullptr );
	}

	void AnimationSystem::Execute( ECS::World& world, float32 elapsed_time )
	{
		AnimationComponentProxy animationProxy;

		std::vector< ECS::GameEntity > entities = world.GetEntitiesOfType< AnimationComponent >();
		auto it = entities.begin();
		for ( ; it != entities.end(); ++it )
		{
			AnimationComponent& animation = it->GetComponent< AnimationComponent >();
			if ( animationProxy.IsPlaying( animation ) )
			{
				animationProxy.AdvanceAnimation( animation, elapsed_time, *_assetManager );

				SpriteRendererComponent& spriteRenderer = it->GetComponent< SpriteRendererComponent >();

				const AnimationClip* currentAnimationClip = animationProxy.GetCurrentAnimation( animation );
				const AnimationAsset* currentAnimationAsset = _assetManager->GetRawAsset< AnimationAsset >(
				    currentAnimationClip->assetHandle, AssetType::ANIMATION );
				assert( currentAnimationAsset != nullptr );

				// TODO We shouldn't be touching like this the texture, width and height variables from sprite renderer
				// components. It is very error prone
				//   Set the right sprite sheet texture for this animation
				AssetHandle textureAssetHandle =
				    _assetManager->GetAsset( currentAnimationAsset->GetSpriteSheetPath(), AssetType::TEXTURE );
				assert( textureAssetHandle.IsValid() );
				const TextureAsset* textureAsset =
				    _assetManager->GetRawAsset< TextureAsset >( textureAssetHandle, AssetType::TEXTURE );
				assert( textureAsset != nullptr );
				spriteRenderer.textureHandler = textureAssetHandle;
				spriteRenderer.type = SpriteType::SPRITE_SHEET;
				spriteRenderer.width = textureAsset->GetWidth();
				spriteRenderer.height = textureAsset->GetHeight();

				// Update sprite renderer to show the current animation frame
				// TODO Add other variables for better flexibility such as initial horizontal pixel, initial vertical
				// pixel, frame width and height
				const uint32 startCurrentFrameXPixel =
				    currentAnimationAsset->GetStartX() +
				    ( currentAnimationAsset->GetFrameWidth() * animationProxy.GetCurrentAnimationFrame( animation ) );
				const uint32 startCurrentFrameYPixel = currentAnimationAsset->GetStartY();
				spriteRenderer.uv0.X( static_cast< float32 >( startCurrentFrameXPixel ) / spriteRenderer.width );
				spriteRenderer.uv0.Y( static_cast< float32 >( startCurrentFrameYPixel ) / spriteRenderer.height );

				spriteRenderer.uv1.X(
				    static_cast< float32 >( startCurrentFrameXPixel + currentAnimationAsset->GetFrameWidth() ) /
				    spriteRenderer.width );
				spriteRenderer.uv1.Y(
				    static_cast< float32 >( startCurrentFrameYPixel + currentAnimationAsset->GetFrameHeight() ) /
				    spriteRenderer.height );
				spriteRenderer.flipX = currentAnimationAsset->IsFlippedX();
			}
		}
	}

	void AnimationSystem::ConfigureAnimationComponent( ECS::GameEntity& entity, const ECS::Prefab& prefab )
	{
		auto component_config_found = prefab.componentConfigurations.find( "Animation" );
		if ( component_config_found == prefab.componentConfigurations.end() )
		{
			return;
		}

		if ( !entity.HasComponent< AnimationComponent >() )
		{
			return;
		}

		const AnimationComponentConfiguration& animation_config =
		    static_cast< const AnimationComponentConfiguration& >( *component_config_found->second );
		AnimationComponent& animation = entity.GetComponent< AnimationComponent >();

		AnimationComponentProxy animationProxy;

		auto cit = animation_config.animations.cbegin();
		for ( ; cit != animation_config.animations.cend(); ++cit )
		{
			AssetHandle animationHandle = _assetManager->GetAsset( cit->path, AssetType::ANIMATION );
			assert( animationHandle.IsValid() );

			const bool addedSuccessfully = animationProxy.AddAnimationClip(
			    animation, AnimationClip( cit->name, _assetManager->GetAsset( cit->path, AssetType::ANIMATION ) ) );
			if ( !addedSuccessfully )
			{
				LOG_WARNING( "Animation clip with name %s is duplicate in prefab %s. Skipping this clip.",
				             cit->name.c_str(), prefab.name.c_str() );
			}
		}

		animationProxy.PlayAnimation( animation, animation_config.initialAnimationName );
	}
} // namespace Engine
