#include "client_player_controller_system.h"

#include "IInputController.h"
#include "ICursor.h"
#include "Vec2f.h"
#include "InputActionIdsConfiguration.h"
#include "InputState.h"
#include "raycaster.h"

#include <vector>

#include "ecs/game_entity.hpp"
#include "ecs/entity_container.h"
#include "ecs/prefab.h"
#include "ecs/world.h"

#include "components/virtual_mouse_component.h"
#include "components/input_component.h"
#include "components/transform_component.h"
#include "components/collider_2d_component.h"
#include "components/player_controller_component.h"
#include "components/health_component.h"

#include "global_components/network_peer_global_component.h"

#include "core/client.h"

ClientPlayerControllerSystem::ClientPlayerControllerSystem( ECS::World* world )
    : ECS::ISimpleSystem()
    , _world( world )
{
}

static void ProcessInputs( ECS::EntityContainer& entityContainer, InputState& outInputState )
{
	const InputComponent& inputComponent = entityContainer.GetGlobalComponent< InputComponent >();

	outInputState.movement.X( inputComponent.inputController->GetAxis( HORIZONTAL_AXIS ) );
	outInputState.movement.Y( inputComponent.inputController->GetAxis( VERTICAL_AXIS ) );
	outInputState.movement.Normalize();

	outInputState.isShooting = inputComponent.cursor->GetButtonPressed( SHOOT_BUTTON );

	const ECS::GameEntity& virtual_mouse_entity = entityContainer.GetFirstEntityOfType< VirtualMouseComponent >();
	const TransformComponent& virtual_mouse_transform = virtual_mouse_entity.GetComponent< TransformComponent >();
	outInputState.virtualMousePosition = virtual_mouse_transform.GetPosition();
}

static void SendInputsToServer( ECS::EntityContainer& entityContainer, const InputState& inputState )
{
	NetworkPeerGlobalComponent& networkPeerComponent =
	    entityContainer.GetGlobalComponent< NetworkPeerGlobalComponent >();

	NetLib::Client& networkClient = *static_cast< NetLib::Client* >( networkPeerComponent.peer );
	networkClient.SendInputs( inputState );
}

void ClientPlayerControllerSystem::Execute( ECS::EntityContainer& entity_container, float32 elapsed_time )
{
	const NetworkPeerGlobalComponent& networkPeerComponent =
	    entity_container.GetGlobalComponent< NetworkPeerGlobalComponent >();
	if ( networkPeerComponent.peer->GetConnectionState() != NetLib::PCS_Connected )
	{
		return;
	}

	InputState inputState;
	ProcessInputs( entity_container, inputState );
	SendInputsToServer( entity_container, inputState );

	ECS::GameEntity local_player = entity_container.GetFirstEntityOfType< PlayerControllerComponent >();
	PlayerControllerComponent& local_player_controller = local_player.GetComponent< PlayerControllerComponent >();

	// Update time left until next shot
	local_player_controller.timeLeftUntilNextShot -= elapsed_time;
	if ( local_player_controller.timeLeftUntilNextShot <= 0.f )
	{
		local_player_controller.timeLeftUntilNextShot = 0.f;
	}

	if ( inputState.isShooting && local_player_controller.timeLeftUntilNextShot == 0.f )
	{
		local_player_controller.timeLeftUntilNextShot = local_player_controller.fireRate;

		const TransformComponent& local_player_transform = local_player.GetComponent< TransformComponent >();

		Raycaster::Ray ray;
		ray.origin = local_player_transform.GetPosition();
		ray.direction = local_player_transform.ConvertRotationAngleToNormalizedDirection();
		ray.maxDistance = 100;

		const std::vector< ECS::GameEntity > entities_with_colliders =
		    entity_container.GetEntitiesOfBothTypes< Collider2DComponent, TransformComponent >();
		const Raycaster::RaycastResult result = Raycaster::ExecuteRaycast( ray, entities_with_colliders, local_player );
		if ( result.entity.IsValid() )
		{
			ECS::GameEntity entity_hit = result.entity;
			if ( entity_hit.HasComponent< HealthComponent >() )
			{
				HealthComponent& health = entity_hit.GetComponent< HealthComponent >();
				health.currentHealth -= 20;
			}
		}

		ECS::GameEntity entity = _world->CreateGameEntity( "Raycast", ray.origin, ray.direction );
	}
}

void ClientPlayerControllerSystem::ConfigurePlayerControllerComponent( ECS::GameEntity& entity,
                                                                       const ECS::Prefab& prefab )
{
	auto component_config_found = prefab.componentConfigurations.find( "PlayerController" );
	if ( component_config_found == prefab.componentConfigurations.end() )
	{
		return;
	}

	if ( !entity.HasComponent< PlayerControllerComponent >() )
	{
		return;
	}

	const PlayerControllerComponentConfiguration& player_controller_config =
	    static_cast< const PlayerControllerComponentConfiguration& >( *component_config_found->second );
	PlayerControllerComponent& player_controller = entity.GetComponent< PlayerControllerComponent >();
	player_controller.movementSpeed = player_controller_config.movementSpeed;
	player_controller.fireRatePerSecond = player_controller_config.fireRatePerSecond;
	player_controller.fireRate = 1.f / player_controller.fireRatePerSecond;
	player_controller.timeLeftUntilNextShot = 0.f;
}
