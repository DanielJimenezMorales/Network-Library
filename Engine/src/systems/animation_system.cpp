#include "animation_system.h"

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

		// Calculate current animation frame
		animation.timeAccumulator += elapsed_time;
		const float32 frameDuration = 1.f / animation.frameRate;
		if ( animation.timeAccumulator >= frameDuration )
		{
			const uint32 numberOfFramesToAdvance = static_cast< uint32 >( animation.timeAccumulator / frameDuration );
			animation.currentFrame = ( animation.currentFrame + numberOfFramesToAdvance ) % animation.numberOfFrames;
			animation.timeAccumulator -= numberOfFramesToAdvance * frameDuration;
		}

		SpriteRendererComponent& spriteRenderer = it->GetComponent< SpriteRendererComponent >();

		// Update sprite renderer to show the current animation frame
		// TODO Add other variables for better flexibility such as initial horizontal pixel, initial vertical pixel,
		// frame width and height
		const uint32 startCurrentFrameXPixel =
		    animation.startFrameXPixel + ( animation.frameWidthPixels * animation.currentFrame );
		const uint32 startCurrentFrameYPixel = animation.startFrameYPixel;
		spriteRenderer.uv0.X( static_cast< float32 >( startCurrentFrameXPixel ) / spriteRenderer.width );
		spriteRenderer.uv0.Y( static_cast< float32 >( startCurrentFrameYPixel ) / spriteRenderer.height );

		spriteRenderer.uv1.X( static_cast< float32 >( startCurrentFrameXPixel + animation.frameWidthPixels ) /
		                      spriteRenderer.width );
		spriteRenderer.uv1.Y( static_cast< float32 >( startCurrentFrameYPixel + animation.frameHeightPixels ) /
		                      spriteRenderer.height );
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
	animation.startFrameXPixel = animation_config.startFrameXPixel;
	animation.startFrameYPixel = animation_config.startFrameYPixel;
	animation.frameWidthPixels = animation_config.frameWidthPixels;
	animation.frameHeightPixels = animation_config.frameHeightPixels;
	animation.frameRate = animation_config.frameRate;
	animation.numberOfFrames = animation_config.numberOfFrames;
	animation.currentFrame = 0;
	animation.timeAccumulator = 0.f;
}
