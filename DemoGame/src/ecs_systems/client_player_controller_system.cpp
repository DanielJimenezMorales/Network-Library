#include "client_player_controller_system.h"

#include "IInputController.h"
#include "ICursor.h"
#include "Vec2f.h"
#include "InputActionIdsConfiguration.h"
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

#include "player_simulation/player_state_utils.h"

#include "core/client.h"

ClientPlayerControllerSystem::ClientPlayerControllerSystem( ECS::World* world )
    : ECS::ISimpleSystem()
    , _world( world )
    , _nextInputStateId( 0 )
    , _predictionBuffer()
{
	InitPredictionBuffer();
}

void ClientPlayerControllerSystem::InitPredictionBuffer()
{
	_predictionBuffer.reserve( MAX_PREDICTION_BUFFER_SIZE );
	for ( uint32 i = 0; i < MAX_PREDICTION_BUFFER_SIZE; ++i )
	{
		_predictionBuffer.emplace_back();
	}
}

void ClientPlayerControllerSystem::ProcessInputs( ECS::EntityContainer& entityContainer, InputState& outInputState )
{
	const InputComponent& inputComponent = entityContainer.GetGlobalComponent< InputComponent >();

	outInputState.movement.X( inputComponent.inputController->GetAxis( HORIZONTAL_AXIS ) );
	outInputState.movement.Y( inputComponent.inputController->GetAxis( VERTICAL_AXIS ) );
	outInputState.movement.Normalize();

	outInputState.isShooting = inputComponent.cursor->GetButtonPressed( SHOOT_BUTTON );

	const ECS::GameEntity& virtual_mouse_entity = entityContainer.GetFirstEntityOfType< VirtualMouseComponent >();
	const TransformComponent& virtual_mouse_transform = virtual_mouse_entity.GetComponent< TransformComponent >();
	outInputState.virtualMousePosition = virtual_mouse_transform.GetPosition();

	outInputState.id = _nextInputStateId;
	++_nextInputStateId;
}

void ClientPlayerControllerSystem::SavePlayerStateInBuffer( const InputState& input_state,
                                                            const PlayerState& player_state )
{
	const uint32 slotIndex = input_state.id % MAX_PREDICTION_BUFFER_SIZE;
	PredictionBufferSlot& slot = _predictionBuffer[ slotIndex ];
	slot.isValid = true;
	slot.inputState = input_state;
	slot.playerState = player_state;
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
	const PlayerStateConfiguration& playerStateConfiguration = local_player_controller.stateConfiguration;

	PlayerState currentPlayerState;
	CreatePlayerStateFromPlayerEntity( local_player, currentPlayerState );

	PlayerState resultPlayerState;
	_playerStateSimulator.Simulate( inputState, currentPlayerState, resultPlayerState, playerStateConfiguration,
	                                elapsed_time );

	SavePlayerStateInBuffer( inputState, resultPlayerState );

	// TODO add this into the player simulator
	//  Update time left until next shot
	local_player_controller.timeLeftUntilNextShot -= elapsed_time;
	if ( local_player_controller.timeLeftUntilNextShot <= 0.f )
	{
		local_player_controller.timeLeftUntilNextShot = 0.f;
	}

	if ( inputState.isShooting && local_player_controller.timeLeftUntilNextShot == 0.f )
	{
		local_player_controller.timeLeftUntilNextShot = local_player_controller.stateConfiguration.GetFireRate();

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
			bool r = true;
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

	const PlayerStateConfiguration playerStateConfig( player_controller_config.movementSpeed,
	                                                  player_controller_config.fireRatePerSecond );

	PlayerControllerComponent& player_controller = entity.GetComponent< PlayerControllerComponent >();
	player_controller.stateConfiguration = playerStateConfig;

	player_controller.timeLeftUntilNextShot = 0.f;
}
