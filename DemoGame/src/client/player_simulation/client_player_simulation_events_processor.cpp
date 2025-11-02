#include "client_player_simulation_events_processor.h"

#include "physics/raycaster.h"
#include "physics/collider_2d_component.h"

#include "transform/transform_component.h"
#include "transform/transform_hierarchy_helper_functions.h"

#include "ecs/world.h"
#include "ecs/game_entity.hpp"

static void OnShotPerformed( Engine::ECS::World& world, const Engine::ECS::GameEntity& player_entity )
{
	const Engine::TransformComponent& localPlayerTransform = player_entity.GetComponent< Engine::TransformComponent >();

	Engine::Raycaster::Ray ray;
	const Engine::TransformComponentProxy transformComponentProxy;
	ray.origin = transformComponentProxy.GetGlobalPosition( localPlayerTransform );
	ray.direction = transformComponentProxy.GetForwardVector( localPlayerTransform );
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

bool ClientPlayerSimulationEventsProcessor::ProcessEvent( Engine::ECS::World& world, Engine::ECS::GameEntity& entity,
                                                          PlayerSimulation::EventType type )
{
	bool result = true;

	if ( type == PlayerSimulation::ON_SHOT_PERFORMED )
	{
		OnShotPerformed( world, entity );
	}
	else
	{
		result = false;
	}

	return result;
}
