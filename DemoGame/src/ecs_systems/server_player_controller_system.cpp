#include "server_player_controller_system.h"

#include "InputState.h"

#include "ecs/game_entity.hpp"
#include "ecs/prefab.h"
#include "ecs/world.h"

#include "components/player_controller_component.h"
#include "components/network_entity_component.h"
#include "components/transform_component.h"

#include "global_components/network_peer_global_component.h"

#include "player_simulation/player_state.h"
#include "player_simulation/player_state_configuration.h"
#include "player_simulation/player_state_utils.h"

ServerPlayerControllerSystem::ServerPlayerControllerSystem()
    : ECS::ISimpleSystem()
    , _playerStateSimulator()
{
}

void ServerPlayerControllerSystem::Execute( ECS::World& world, float32 elapsed_time )
{
	NetworkPeerGlobalComponent& networkPeerComponent = world.GetGlobalComponent< NetworkPeerGlobalComponent >();
	NetLib::Server* serverPeer = networkPeerComponent.GetPeerAsServer();

	std::vector< ECS::GameEntity > entities = world.GetEntitiesOfType< PlayerControllerComponent >();
	for ( auto it = entities.begin(); it != entities.end(); ++it )
	{
		const NetworkEntityComponent& networkEntityComponent = it->GetComponent< NetworkEntityComponent >();
		const NetLib::IInputState* baseInputState =
		    serverPeer->GetInputFromRemotePeer( networkEntityComponent.controlledByPeerId );
		if ( baseInputState == nullptr )
		{
			return;
		}

		const InputState* inputState = static_cast< const InputState* >( baseInputState );

		const PlayerControllerComponent& playerController = it->GetComponent< PlayerControllerComponent >();
		const PlayerStateConfiguration& playerStateConfiguration = playerController.stateConfiguration;

		const PlayerState currentPlayerState = GetPlayerStateFromPlayerEntity( *it, inputState->tick );
		_playerStateSimulator.Configure( world, *it );
		const PlayerState resultPlayerState =
		    _playerStateSimulator.Simulate( *inputState, currentPlayerState, playerStateConfiguration, elapsed_time );
		ApplyPlayerStateToPlayerEntity( *it, resultPlayerState );
	}
}

void ServerPlayerControllerSystem::ConfigurePlayerControllerComponent( ECS::GameEntity& entity,
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
