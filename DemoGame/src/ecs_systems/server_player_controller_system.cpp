#include "server_player_controller_system.h"

#include "GameEntity.hpp"
#include "InputState.h"
#include "PlayerSimulator.h"

#include "ecs/entity_container.h"

#include "components/player_controller_component.h"
#include "components/network_peer_component.h"
#include "components/network_entity_component.h"

ServerPlayerControllerSystem::ServerPlayerControllerSystem()
    : ECS::ISimpleSystem()
{
}

void ServerPlayerControllerSystem::Execute( GameEntity& entity, float32 elapsed_time )
{
	ECS::EntityContainer* entity_container = entity.GetEntityContainer();

	GameEntity& networkPeerEntity = entity_container->GetFirstEntityOfType< NetworkPeerComponent >();
	NetworkPeerComponent& networkPeerComponent = networkPeerEntity.GetComponent< NetworkPeerComponent >();
	NetLib::Server* serverPeer = networkPeerComponent.GetPeerAsServer();

	const NetworkEntityComponent& networkEntityComponent = entity.GetComponent< NetworkEntityComponent >();
	const NetLib::IInputState* baseInputState =
	    serverPeer->GetInputFromRemotePeer( networkEntityComponent.controlledByPeerId );
	if ( baseInputState == nullptr )
	{
		return;
	}

	const InputState* inputState = static_cast< const InputState* >( baseInputState );
	PlayerSimulator::Simulate( *inputState, entity, elapsed_time );
}
