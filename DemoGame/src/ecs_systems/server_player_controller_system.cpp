#include "server_player_controller_system.h"

#include "GameEntity.hpp"
#include "InputState.h"
#include "PlayerSimulator.h"

#include "ecs/entity_container.h"
#include "ecs/prefab.h"

#include "components/player_controller_component.h"
#include "components/network_entity_component.h"

#include "global_components/network_peer_global_component.h"

ServerPlayerControllerSystem::ServerPlayerControllerSystem()
    : ECS::ISimpleSystem()
{
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
		PlayerSimulator::Simulate( *inputState, *it, elapsed_time );
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
	PlayerControllerComponent& player_controller = entity.GetComponent< PlayerControllerComponent >();
	player_controller.movementSpeed = player_controller_config.movementSpeed;
}
