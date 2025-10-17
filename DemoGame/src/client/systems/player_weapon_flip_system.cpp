#include "player_weapon_flip_system.h"

#include "ecs/world.h"
#include "ecs/game_entity.hpp"

#include "components/sprite_renderer_component.h"
#include "components/transform_component.h"

#include "transform/transform_hierarchy_helper_functions.h"

#include "math_utils.h"

#include "client/components/player_visual_weapon_tag_component.h"

#include <cassert>

void PlayerWeaponFlipSystem::Execute( Engine::ECS::World& world, float32 elapsed_time )
{
	const Engine::TransformComponentProxy transformComponentProxy;

	std::vector< Engine::ECS::GameEntity > entities = world.GetEntitiesOfType< PlayerVisualWeaponTagComponent >();
	auto it = entities.begin();
	for ( ; it != entities.end(); ++it )
	{
		assert( it->HasComponent< Engine::SpriteRendererComponent >() );

		const Engine::TransformComponent& transform = it->GetComponent< Engine::TransformComponent >();
		Engine::SpriteRendererComponent& spriteRenderer = it->GetComponent< Engine::SpriteRendererComponent >();

		const Vec2f forwardDirection = transformComponentProxy.GetForwardVector( transform );
		spriteRenderer.flipY = forwardDirection.X() < 0.f;
	}
}
