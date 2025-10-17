#include "player_weapon_visibility_system.h"

#include "ecs/world.h"
#include "ecs/game_entity.hpp"

#include "components/sprite_renderer_component.h"
#include "components/transform_component.h"

#include "transform/transform_hierarchy_helper_functions.h"

#include "shared/components/player_controller_component.h"

#include "client/components/ghost_object_component.h"
#include "client/components/player_visual_weapon_tag_component.h"

void PlayerWeaponVisibilitySystem::Execute( Engine::ECS::World& world, float32 elapsed_time )
{
	const Engine::TransformComponentProxy transformComponentProxy;

	std::vector< Engine::ECS::GameEntity > entities = world.GetEntitiesOfType< PlayerVisualWeaponTagComponent >();
	for ( auto it = entities.begin(); it != entities.end(); ++it )
	{
		const Engine::TransformComponent& transform = it->GetComponent< Engine::TransformComponent >();
		const Engine::ECS::GameEntity& playerAimEntity = transformComponentProxy.GetParent( transform );

		const Engine::TransformComponent& playerAimTransform =
		    playerAimEntity.GetComponent< Engine::TransformComponent >();
		const Engine::ECS::GameEntity& playerInterpolatedEntity =
		    transformComponentProxy.GetParent( playerAimTransform );

		const GhostObjectComponent& ghostObject = playerInterpolatedEntity.GetComponent< GhostObjectComponent >();
		const Engine::ECS::GameEntity& playerGhostEntity = ghostObject.entity;
		const PlayerControllerComponent& playerController =
		    playerGhostEntity.GetComponent< PlayerControllerComponent >();

		Engine::SpriteRendererComponent& spriteRenderer = it->GetComponent< Engine::SpriteRendererComponent >();
		spriteRenderer.isDisabled = !playerController.isAiming;
	}
}
