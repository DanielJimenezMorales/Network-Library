#include "client_player_simulation_callbacks.h"

#include "raycaster.h"

#include "components/transform_component.h"
#include "components/collider_2d_component.h"

#include "ecs/world.h"
#include "ecs/game_entity.hpp"

void OnShotPerformed( Engine::ECS::World& world, const Engine::ECS::GameEntity& player_entity )
{
	const Engine::TransformComponent& local_player_transform =
	    player_entity.GetComponent< Engine::TransformComponent >();

	Engine::Raycaster::Ray ray;
	ray.origin = local_player_transform.GetPosition();
	ray.direction = local_player_transform.ConvertRotationAngleToNormalizedDirection();
	ray.maxDistance = 100;

	const std::vector< Engine::ECS::GameEntity > entities_with_colliders =
	    world.GetEntitiesOfBothTypes< Engine::Collider2DComponent, Engine::TransformComponent >();
	const Engine::Raycaster::RaycastResult result =
	    Engine::Raycaster::ExecuteRaycast( ray, entities_with_colliders, player_entity );
	if ( result.entity.IsValid() )
	{
		bool r = true;
	}

	Engine::ECS::GameEntity entity = world.CreateGameEntity( "Raycast", ray.origin, ray.direction );
}
