#include "client_local_player_predictor_system.h"

#include "IInputController.h"
#include "ICursor.h"
#include "Vec2f.h"
#include "InputActionIdsConfiguration.h"
#include "raycaster.h"
#include "client_side_prediction_buffer_slot.h"
#include "InputState.h"

#include "ecs/game_entity.hpp"
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

ClientLocalPlayerPredictorSystem::ClientLocalPlayerPredictorSystem( ECS::World* world )
    : ECS::ISimpleSystem()
    , _world( world )
    , _nextInputStateId( 0 )
{
	SubscribeToSimulationCallbacks();
}

static void ProcessInputs( ECS::World& world, InputState& outInputState )
{
	const InputComponent& inputComponent = world.GetGlobalComponent< InputComponent >();

	outInputState.movement.X( inputComponent.inputController->GetAxis( HORIZONTAL_AXIS ) );
	outInputState.movement.Y( inputComponent.inputController->GetAxis( VERTICAL_AXIS ) );
	outInputState.movement.Normalize();

	outInputState.isShooting = inputComponent.cursor->GetButtonPressed( SHOOT_BUTTON );

	const ECS::GameEntity& virtual_mouse_entity = world.GetFirstEntityOfType< VirtualMouseComponent >();
	const TransformComponent& virtual_mouse_transform = virtual_mouse_entity.GetComponent< TransformComponent >();
	outInputState.virtualMousePosition = virtual_mouse_transform.GetPosition();
}

static InputState GetInputState( ECS::World& world, uint32 current_tick, float32 elapsed_time )
{
	InputState inputState;
	inputState.tick = current_tick;
	ProcessInputs( world, inputState );
	return inputState;
}

void ClientLocalPlayerPredictorSystem::OnShotPerformedCallback()
{
	OnShotPerformed( *_world, _currentPlayerEntityBeingProcessed );
}

void ClientLocalPlayerPredictorSystem::SubscribeToSimulationCallbacks()
{
	auto onShotPerformedCallback = std::bind( &ClientLocalPlayerPredictorSystem::OnShotPerformedCallback, this );
	_playerStateSimulator.SubscribeToOnShotPerformed( onShotPerformedCallback );
}

static void SavePlayerStateInBuffer( ClientSidePredictionComponent& client_side_prediction_component,
                                     const InputState& input_state, const PlayerState& resulted_player_state,
                                     float32 elapsed_time )
{
	const uint32 slotIndex = client_side_prediction_component.ConvertTickToBufferSlotIndex( input_state.tick );
	client_side_prediction_component.inputStatesBuffer[ slotIndex ] = input_state;
	client_side_prediction_component.elapsedTimeBuffer[ slotIndex ] = elapsed_time;
	client_side_prediction_component.resultedPlayerStatesBuffer[ slotIndex ] = resulted_player_state;
}

static void SendInputsToServer( ECS::World& world, const InputState& inputState )
{
	NetworkPeerGlobalComponent& networkPeerComponent = world.GetGlobalComponent< NetworkPeerGlobalComponent >();

	NetLib::Client& networkClient = *static_cast< NetLib::Client* >( networkPeerComponent.peer );
	networkClient.SendInputs( inputState );
}

void ClientLocalPlayerPredictorSystem::ExecuteLocalPrediction( ECS::GameEntity& entity, const InputState& input_state,
                                                               float32 elapsed_time )
{
	_currentPlayerEntityBeingProcessed = entity;

	// Get the current state and the configuration for the predicted entity
	const PlayerState currentState = GetPlayerStateFromPlayerEntity( entity, input_state.tick );

	PlayerControllerComponent& localPlayerController = entity.GetComponent< PlayerControllerComponent >();
	const PlayerStateConfiguration& playerStateConfiguration = localPlayerController.stateConfiguration;

	// Simulate the player logic locally and get the resulted simulation state
	const PlayerState resultPlayerState =
	    _playerStateSimulator.Simulate( input_state, currentState, playerStateConfiguration, elapsed_time );
	_currentPlayerEntityBeingProcessed = ECS::GameEntity();

	// Store the data in the prediction buffer in case we need to reconcile with the server later.
	ClientSidePredictionComponent& clientSidePredictionComponent =
	    entity.GetComponent< ClientSidePredictionComponent >();
	SavePlayerStateInBuffer( clientSidePredictionComponent, input_state, resultPlayerState, elapsed_time );

	// Apply the resulted simulation state to the entity
	ApplyPlayerStateToPlayerEntity( entity, resultPlayerState );
}

void ClientLocalPlayerPredictorSystem::Execute( ECS::World& world, float32 elapsed_time )
{
	const NetworkPeerGlobalComponent& networkPeerComponent = world.GetGlobalComponent< NetworkPeerGlobalComponent >();
	if ( networkPeerComponent.peer->GetConnectionState() != NetLib::PCS_Connected )
	{
		return;
	}

	// Get the current network simulation tick
	const uint32 currentTick = networkPeerComponent.peer->GetCurrentTick();

	// Get the current input state and send it to the server
	const InputState inputState = GetInputState( world, currentTick, elapsed_time );
	SendInputsToServer( world, inputState );

	// Predict the next simulation state based on the inputs for each predicted entity
	std::vector< ECS::GameEntity > localPredictedEntities = world.GetEntitiesOfType< ClientSidePredictionComponent >();
	for ( auto it = localPredictedEntities.begin(); it != localPredictedEntities.end(); ++it )
	{
		ExecuteLocalPrediction( *it, inputState, elapsed_time );
	}
}

void ClientLocalPlayerPredictorSystem::ConfigurePlayerControllerComponent( ECS::GameEntity& entity,
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

void ClientLocalPlayerPredictorSystem::ConfigureClientSidePredictorComponent( ECS::GameEntity& entity,
                                                                              const ECS::Prefab& prefab )
{
	if ( !entity.HasComponent< ClientSidePredictionComponent >() )
	{
		return;
	}

	ClientSidePredictionComponent& clientSidePredictor = entity.GetComponent< ClientSidePredictionComponent >();

	clientSidePredictor.inputStatesBuffer.reserve( clientSidePredictor.MAX_PREDICTION_BUFFER_SIZE );
	clientSidePredictor.elapsedTimeBuffer.reserve( clientSidePredictor.MAX_PREDICTION_BUFFER_SIZE );
	clientSidePredictor.resultedPlayerStatesBuffer.reserve( clientSidePredictor.MAX_PREDICTION_BUFFER_SIZE );
	for ( uint32 i = 0; i < clientSidePredictor.MAX_PREDICTION_BUFFER_SIZE; ++i )
	{
		clientSidePredictor.inputStatesBuffer.emplace_back();
		clientSidePredictor.elapsedTimeBuffer.emplace_back();
		clientSidePredictor.resultedPlayerStatesBuffer.emplace_back();
	}
}
