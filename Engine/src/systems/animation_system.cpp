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
		const uint32 totalAnimationFramesPixels = animation.pixelsPerFrame * animation.numberOfFrames;
		const uint32 startCurrentFramePixel = animation.pixelsPerFrame * animation.currentFrame;
		spriteRenderer.uv0.X( static_cast< float32 >( startCurrentFramePixel ) / spriteRenderer.width );
		spriteRenderer.uv0.Y( 0.2f ); // TODO Do not hardcode this

		spriteRenderer.uv1.X( static_cast< float32 >( startCurrentFramePixel + animation.pixelsPerFrame ) /
		                      spriteRenderer.width );
		spriteRenderer.uv1.Y( 0.4f ); // TODO Do not hardcode this
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
	animation.currentFrame = 0;
	animation.frameRate = animation_config.frameRate;
	animation.numberOfFrames = animation_config.numberOfFrames;
	animation.pixelsPerFrame = animation_config.pixelsPerFrame;
	animation.timeAccumulator = 0.f;
}
