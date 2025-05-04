#include "client_player_controller_system.h"

#include "IInputController.h"
#include "ICursor.h"
#include "Vec2f.h"
#include "InputActionIdsConfiguration.h"
#include "raycaster.h"
#include "client_side_prediction_buffer_slot.h"

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
#include "components/client_side_prediction_component.h"

#include "global_components/network_peer_global_component.h"

#include "player_simulation/player_state_utils.h"
#include "player_simulation/client_player_simulation_callbacks.h"

#include "core/client.h"

ClientPlayerControllerSystem::ClientPlayerControllerSystem( ECS::World* world )
    : ECS::ISimpleSystem()
    , _world( world )
    , _nextInputStateId( 0 )
{
	SubscribeToSimulationCallbacks();
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

static InputState GetInputState( ECS::EntityContainer& entityContainer, uint32 current_tick, float32 elapsed_time )
{
	InputState inputState;
	inputState.tick = current_tick;
	ProcessInputs( entityContainer, inputState );
	return inputState;
}

void ClientPlayerControllerSystem::OnShotPerformedCallback()
{
	OnShotPerformed( *_world, _currentPlayerEntityBeingProcessed );
}

void ClientPlayerControllerSystem::SubscribeToSimulationCallbacks()
{
	auto onShotPerformedCallback = std::bind( &ClientPlayerControllerSystem::OnShotPerformedCallback, this );
	_playerStateSimulator.SubscribeToOnShotPerformed( onShotPerformedCallback );
}

void ClientPlayerControllerSystem::SavePlayerStateInBuffer(
    ClientSidePredictionComponent& client_side_prediction_component, const InputState& input_state,
    const PlayerState& player_state )
{
	const uint32 slotIndex = input_state.tick % client_side_prediction_component.MAX_PREDICTION_BUFFER_SIZE;
	client_side_prediction_component.inputStatesBuffer[ slotIndex ] = input_state;
	client_side_prediction_component.playerStatesBuffer[ slotIndex ] = player_state;
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

	const uint32 currentTick = networkPeerComponent.peer->GetCurrentTick();

	const InputState inputState = GetInputState( entity_container, currentTick, elapsed_time );
	SendInputsToServer( entity_container, inputState );

	ECS::GameEntity local_player = entity_container.GetFirstEntityOfType< PlayerControllerComponent >();
	_currentPlayerEntityBeingProcessed = local_player;

	const PlayerState currentPlayerState = GetPlayerStateFromPlayerEntity( local_player, currentTick );

	ClientSidePredictionComponent& clientSidePredictionComponent =
	    local_player.GetComponent< ClientSidePredictionComponent >();
	SavePlayerStateInBuffer( clientSidePredictionComponent, inputState, currentPlayerState );

	PlayerControllerComponent& local_player_controller = local_player.GetComponent< PlayerControllerComponent >();
	const PlayerStateConfiguration& playerStateConfiguration = local_player_controller.stateConfiguration;
	const PlayerState resultPlayerState =
	    _playerStateSimulator.Simulate( inputState, currentPlayerState, playerStateConfiguration, elapsed_time );
	_currentPlayerEntityBeingProcessed = ECS::GameEntity();
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

void ClientPlayerControllerSystem::ConfigureClientSidePredictorComponent( ECS::GameEntity& entity,
                                                                          const ECS::Prefab& prefab )
{
	if ( !entity.HasComponent< ClientSidePredictionComponent >() )
	{
		return;
	}

	ClientSidePredictionComponent& clientSidePredictor = entity.GetComponent< ClientSidePredictionComponent >();

	clientSidePredictor.inputStatesBuffer.reserve( clientSidePredictor.MAX_PREDICTION_BUFFER_SIZE );
	clientSidePredictor.playerStatesBuffer.reserve( clientSidePredictor.MAX_PREDICTION_BUFFER_SIZE );
	for ( uint32 i = 0; i < clientSidePredictor.MAX_PREDICTION_BUFFER_SIZE; ++i )
	{
		clientSidePredictor.inputStatesBuffer.emplace_back();
		clientSidePredictor.playerStatesBuffer.emplace_back();
	}
}
