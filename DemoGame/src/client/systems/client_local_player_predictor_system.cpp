#include "client_local_player_predictor_system.h"

#include "inputs/i_input_controller.h"
#include "inputs/i_cursor.h"

#include "vec2f.h"
#include "raycaster.h"
#include "shared/InputActionIdsConfiguration.h"
#include "shared/InputState.h"

#include "ecs/game_entity.hpp"
#include "ecs/prefab.h"
#include "ecs/world.h"

#include "components/collider_2d_component.h"
#include "components/transform_component.h"

#include "transform/transform_hierarchy_helper_functions.h"

#include "shared/components/player_controller_component.h"
#include "shared/components/health_component.h"

#include "client/components/virtual_mouse_component.h"
#include "client/components/client_side_prediction_component.h"

#include "shared/global_components/network_peer_global_component.h"
#include "shared/global_components/network_peer_global_component.h"
#include "global_components/input_handler_global_component.h"

#include "shared/player_simulation/player_state_utils.h"

#include "core/client.h"

ClientLocalPlayerPredictorSystem::ClientLocalPlayerPredictorSystem( Engine::ECS::World* world )
    : Engine::ECS::ISimpleSystem()
    , _world( world )
    , _nextInputStateId( 0 )
    , _playerStateSimulator()
    , _simulationEventsProcessor()
{
}

static void ProcessInputs( Engine::ECS::World& world, InputState& outInputState )
{
	const NetworkPeerGlobalComponent& networkPeerComponent = world.GetGlobalComponent< NetworkPeerGlobalComponent >();
	outInputState.serverTime = networkPeerComponent.peer->GetServerTime();

	const Engine::InputHandlerGlobalComponent& inputHandler =
	    world.GetGlobalComponent< Engine::InputHandlerGlobalComponent >();

	outInputState.movement.X( inputHandler.ControllerGetAxis( KEYBOARD_NAME, HORIZONTAL_AXIS ) );
	outInputState.movement.Y( inputHandler.ControllerGetAxis( KEYBOARD_NAME, VERTICAL_AXIS ) );
	outInputState.movement.Normalize();

	outInputState.isShooting = inputHandler.CursorGetButtonPressed( MOUSE_NAME, SHOOT_BUTTON );

	const Engine::ECS::GameEntity& virtualMousEentity = world.GetFirstEntityOfType< VirtualMouseComponent >();
	const Engine::TransformComponent& virtualMouseTransform =
	    virtualMousEentity.GetComponent< Engine::TransformComponent >();
	// TODO instead of sending a position as an input, we should send the delta from the mouse. As the position can be
	// easily hackable.
	const Engine::TransformComponentProxy transformComponentProxy;
	outInputState.virtualMousePosition = transformComponentProxy.GetGlobalPosition( virtualMouseTransform );
}

static InputState GetInputState( Engine::ECS::World& world, uint32 current_tick, float32 elapsed_time )
{
	InputState inputState;
	inputState.tick = current_tick;
	ProcessInputs( world, inputState );
	return inputState;
}

static void SavePlayerStateInBuffer( ClientSidePredictionComponent& client_side_prediction_component,
                                     const InputState& input_state,
                                     const PlayerSimulation::PlayerState& resulted_player_state, float32 elapsed_time )
{
	const uint32 slotIndex = client_side_prediction_component.ConvertTickToBufferSlotIndex( input_state.tick );
	client_side_prediction_component.inputStatesBuffer[ slotIndex ] = input_state;
	client_side_prediction_component.elapsedTimeBuffer[ slotIndex ] = elapsed_time;
	client_side_prediction_component.resultedPlayerStatesBuffer[ slotIndex ] = resulted_player_state;
}

static void SendInputsToServer( Engine::ECS::World& world, const InputState& inputState )
{
	NetworkPeerGlobalComponent& networkPeerComponent = world.GetGlobalComponent< NetworkPeerGlobalComponent >();

	NetLib::Client& networkClient = *static_cast< NetLib::Client* >( networkPeerComponent.peer );
	networkClient.SendInputs( inputState );
}

void ClientLocalPlayerPredictorSystem::ExecuteLocalPrediction( Engine::ECS::GameEntity& entity,
                                                               const InputState& input_state, float32 elapsed_time )
{
	// Get the current state and the configuration for the predicted entity
	const PlayerSimulation::PlayerState currentState =
	    PlayerSimulation::GetPlayerStateFromPlayerEntity( entity, input_state.tick );

	PlayerControllerComponent& localPlayerController = entity.GetComponent< PlayerControllerComponent >();
	const PlayerSimulation::PlayerStateConfiguration& playerStateConfiguration =
	    localPlayerController.stateConfiguration;

	// Simulate the player logic locally and get the resulted simulation state
	const PlayerSimulation::PlayerState resultPlayerState =
	    _playerStateSimulator.Simulate( input_state, currentState, playerStateConfiguration, elapsed_time );

	// Store the data in the prediction buffer in case we need to reconcile with the server later.
	ClientSidePredictionComponent& clientSidePredictionComponent =
	    entity.GetComponent< ClientSidePredictionComponent >();
	SavePlayerStateInBuffer( clientSidePredictionComponent, input_state, resultPlayerState, elapsed_time );

	// Apply the resulted simulation state to the entity
	PlayerSimulation::ApplyPlayerStateToPlayerEntity( entity, resultPlayerState );

	// Fire simulation events
	_playerStateSimulator.ProcessLastSimulationEvents( *_world, entity, &_simulationEventsProcessor );
}

void ClientLocalPlayerPredictorSystem::Execute( Engine::ECS::World& world, float32 elapsed_time )
{
	const NetworkPeerGlobalComponent& networkPeerComponent = world.GetGlobalComponent< NetworkPeerGlobalComponent >();
	if ( networkPeerComponent.peer->GetConnectionState() != NetLib::PCS_Connected )
	{
		return;
	}

	std::vector< Engine::ECS::GameEntity > localPredictedEntities =
	    world.GetEntitiesOfType< ClientSidePredictionComponent >();

	// Check if there's any entity that needs to be predicted
	if ( localPredictedEntities.empty() )
	{
		return;
	}

	// Get the current network simulation tick
	const uint32 currentTick = networkPeerComponent.peer->GetCurrentTick();

	// Get the current input state and send it to the server
	const InputState inputState = GetInputState( world, currentTick, elapsed_time );
	SendInputsToServer( world, inputState );

	// Predict the next simulation state based on the inputs for each predicted entity
	for ( auto it = localPredictedEntities.begin(); it != localPredictedEntities.end(); ++it )
	{
		ExecuteLocalPrediction( *it, inputState, elapsed_time );
	}
}

void ClientLocalPlayerPredictorSystem::ConfigurePlayerControllerComponent( Engine::ECS::GameEntity& entity,
                                                                           const Engine::ECS::Prefab& prefab )
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

	const PlayerSimulation::PlayerStateConfiguration playerStateConfig( player_controller_config.movementSpeed,
	                                                                    player_controller_config.fireRatePerSecond );

	PlayerControllerComponent& player_controller = entity.GetComponent< PlayerControllerComponent >();
	player_controller.stateConfiguration = playerStateConfig;

	player_controller.timeLeftUntilNextShot = 0.f;
}

void ClientLocalPlayerPredictorSystem::ConfigureClientSidePredictorComponent( Engine::ECS::GameEntity& entity,
                                                                              const Engine::ECS::Prefab& prefab )
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
