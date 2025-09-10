#include "animation_system.h"

#include "logger.h"

#include "ecs/world.h"
#include "ecs/game_entity.hpp"
#include "ecs/prefab.h"

#include "components/animation_component.h"
#include "components/sprite_renderer_component.h"

#include "component_configurations/animation_component_configuration.h"

void Engine::AnimationSystem::Execute( ECS::World& world, float32 elapsed_time )
{
	std::vector< ECS::GameEntity > entities = world.GetEntitiesOfType< AnimationComponent >();
	auto it = entities.begin();
	for ( ; it != entities.end(); ++it )
	{
		AnimationComponent& animation = it->GetComponent< AnimationComponent >();
		if ( animation.currentAnimation != nullptr )
		{
			const AnimationClip& currentAnimation = *animation.currentAnimation;

			// Calculate current animation frame
			animation.timeAccumulator += elapsed_time;
			const float32 frameDuration = 1.f / currentAnimation.frameRate;
			if ( animation.timeAccumulator >= frameDuration )
			{
				const uint32 numberOfFramesToAdvance =
				    static_cast< uint32 >( animation.timeAccumulator / frameDuration );
				animation.currentFrame =
				    ( animation.currentFrame + numberOfFramesToAdvance ) % currentAnimation.numberOfFrames;
				animation.timeAccumulator -= numberOfFramesToAdvance * frameDuration;
			}

			SpriteRendererComponent& spriteRenderer = it->GetComponent< SpriteRendererComponent >();
			assert( spriteRenderer.type == SpriteType::SPRITE_SHEET );

			// Update sprite renderer to show the current animation frame
			// TODO Add other variables for better flexibility such as initial horizontal pixel, initial vertical pixel,
			// frame width and height
			const uint32 startCurrentFrameXPixel =
			    currentAnimation.startFrameXPixel + ( currentAnimation.frameWidthPixels * animation.currentFrame );
			const uint32 startCurrentFrameYPixel = currentAnimation.startFrameYPixel;
			spriteRenderer.uv0.X( static_cast< float32 >( startCurrentFrameXPixel ) / spriteRenderer.width );
			spriteRenderer.uv0.Y( static_cast< float32 >( startCurrentFrameYPixel ) / spriteRenderer.height );

			spriteRenderer.uv1.X(
			    static_cast< float32 >( startCurrentFrameXPixel + currentAnimation.frameWidthPixels ) /
			    spriteRenderer.width );
			spriteRenderer.uv1.Y(
			    static_cast< float32 >( startCurrentFrameYPixel + currentAnimation.frameHeightPixels ) /
			    spriteRenderer.height );
		}
	}
}

void Engine::AnimationSystem::ConfigureAnimationComponent( ECS::GameEntity& entity, const ECS::Prefab& prefab )
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

	auto cit = animation_config.animations.cbegin();
	for ( ; cit != animation_config.animations.cend(); ++cit )
	{
		if ( animation.animations.find( cit->name ) == animation.animations.end() )
		{
			animation.animations[ cit->name ] = *cit;
		}
		else
		{
			LOG_WARNING( "Animation clip with name %s is duplicate in prefab %s. Skipping this clip.",
			             cit->name.c_str(), prefab.name.c_str() );
		}
	}

	animation.currentAnimation = &animation.animations[ animation_config.initialAnimationName ];
	animation.currentFrame = 0;
	animation.timeAccumulator = 0.f;
}
