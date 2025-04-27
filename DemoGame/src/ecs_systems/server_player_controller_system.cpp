#include "server_player_controller_system.h"

#include "InputState.h"

#include "ecs/game_entity.hpp"
#include "ecs/entity_container.h"
#include "ecs/prefab.h"
#include "ecs/world.h"

#include "components/player_controller_component.h"
#include "components/network_entity_component.h"
#include "components/transform_component.h"

#include "global_components/network_peer_global_component.h"

#include "player_simulation/player_state.h"
#include "player_simulation/player_state_configuration.h"

ServerPlayerControllerSystem::ServerPlayerControllerSystem( ECS::World* world )
    : ECS::ISimpleSystem()
    , _playerStateSimulator( world )
{
}

static void CreatePlayerState( const ECS::GameEntity& player_entity, PlayerState& player_state )
{
	const TransformComponent& transform = player_entity.GetComponent< TransformComponent >();
	player_state.position = transform.GetPosition();
	player_state.rotationAngle = transform.GetRotationAngle();
}

static void ApplyPlayerState( ECS::GameEntity& player_entity, const PlayerState& player_state )
{
	TransformComponent& transform = player_entity.GetComponent< TransformComponent >();
	transform.SetPosition( player_state.position );
	transform.SetRotationAngle( player_state.rotationAngle );
}

void ServerPlayerControllerSystem::Execute( ECS::EntityContainer& entity_container, float32 elapsed_time )
{
	NetworkPeerGlobalComponent& networkPeerComponent =
	    entity_container.GetGlobalComponent< NetworkPeerGlobalComponent >();
	NetLib::Server* serverPeer = networkPeerComponent.GetPeerAsServer();

	std::vector< ECS::GameEntity > entities = entity_container.GetEntitiesOfType< PlayerControllerComponent >();
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

		PlayerState currentPlayerState;
		CreatePlayerState( *it, currentPlayerState );
		PlayerState resultPlayerState;
		_playerStateSimulator.Simulate( *inputState, currentPlayerState, resultPlayerState, playerStateConfiguration,
		                                elapsed_time );
		ApplyPlayerState( *it, resultPlayerState );
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

	const PlayerStateConfiguration playerStateConfig(player_controller_config.movementSpeed, player_controller_config.fireRatePerSecond);

	PlayerControllerComponent& player_controller = entity.GetComponent< PlayerControllerComponent >();
	player_controller.stateConfiguration = playerStateConfig;

	player_controller.timeLeftUntilNextShot = 0.f;
}
