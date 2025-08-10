#include "server_player_controller_system.h"

#include <cassert>

#include "shared/InputState.h"

#include "ecs/game_entity.hpp"
#include "ecs/prefab.h"
#include "ecs/world.h"

#include "components/transform_component.h"

#include "shared/components/player_controller_component.h"
#include "shared/components/network_entity_component.h"

#include "server/components/server_player_state_storage_component.h"

#include "shared/global_components/network_peer_global_component.h"

#include "shared/player_simulation/player_state.h"
#include "shared/player_simulation/player_state_configuration.h"
#include "shared/player_simulation/player_state_utils.h"
#include "shared/player_simulation/player_state_simulator.h"

ServerPlayerControllerSystem::ServerPlayerControllerSystem()
    : Engine::ECS::ISimpleSystem()
{
}

static const InputState* GetInputForPlayer( const Engine::ECS::GameEntity& entity, NetLib::Server* server_peer )
{
	assert( entity.HasComponent< NetworkEntityComponent >() );

	// Get the network entity componet in order to get the controlled peer id and be able to grab the current inputs
	// to simulate
	const NetworkEntityComponent& networkEntityComponent = entity.GetComponent< NetworkEntityComponent >();
	const NetLib::IInputState* baseInputState =
	    server_peer->GetInputFromRemotePeer( networkEntityComponent.controlledByPeerId );

	// If we don't have any input state from the remote peer we skip this entity
	// TODO Server should always have inputs to simulate. If no inputs have arrived from server duplicate and return
	// the last known input from this peer
	if ( baseInputState == nullptr )
	{
		return nullptr;
	}

	return static_cast< const InputState* >( baseInputState );
}

static void ExecutePlayerSimulation( Engine::ECS::GameEntity& entity, const InputState& input_state,
                                     float32 elapsed_time )
{
	// Get all data needed for the simulation
	const PlayerControllerComponent& playerController = entity.GetComponent< PlayerControllerComponent >();
	const PlayerStateConfiguration& playerStateConfiguration = playerController.stateConfiguration;
	const PlayerState currentPlayerState = GetPlayerStateFromPlayerEntity( entity, input_state.tick );
	PlayerSimulation::PlayerStateSimulator playerStateSimulator;

	// Simulate the player logic and get the resulted simulation state
	const PlayerState resultPlayerState =
	    playerStateSimulator.Simulate( input_state, currentPlayerState, playerStateConfiguration, elapsed_time );

	// Apply the resulted simulation state to the entity
	ApplyPlayerStateToPlayerEntity( entity, resultPlayerState );

	// Save resulted simulation state in order to recover all its info when serializing it to send it to the target
	// client
	ServerPlayerStateStorageComponent& serverPlayerStateStorage =
	    entity.GetComponent< ServerPlayerStateStorageComponent >();
	serverPlayerStateStorage.lastPlayerStateSimulated = resultPlayerState;
}

void ServerPlayerControllerSystem::Execute( Engine::ECS::World& world, float32 elapsed_time )
{
	NetworkPeerGlobalComponent& networkPeerComponent = world.GetGlobalComponent< NetworkPeerGlobalComponent >();
	NetLib::Server* serverPeer = networkPeerComponent.GetPeerAsServer();

	std::vector< Engine::ECS::GameEntity > entities = world.GetEntitiesOfType< PlayerControllerComponent >();
	for ( auto it = entities.begin(); it != entities.end(); ++it )
	{
		// Get the input state for the current player.
		const InputState* inputState = GetInputForPlayer( *it, serverPeer );
		if ( inputState == nullptr )
		{
			LOG_WARNING( "No input state found for player. Skipping its simulation..." );
			continue;
		}

		// Execute player simulation based on inputs
		ExecutePlayerSimulation( *it, *inputState, elapsed_time );
	}
}

void ServerPlayerControllerSystem::ConfigurePlayerControllerComponent( Engine::ECS::GameEntity& entity,
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

	const PlayerStateConfiguration playerStateConfig( player_controller_config.movementSpeed,
	                                                  player_controller_config.fireRatePerSecond );

	PlayerControllerComponent& player_controller = entity.GetComponent< PlayerControllerComponent >();
	player_controller.stateConfiguration = playerStateConfig;

	player_controller.timeLeftUntilNextShot = 0.f;
}
