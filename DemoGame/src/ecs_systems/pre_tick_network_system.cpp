#include "pre_tick_network_system.h"

#include "GameEntity.hpp"

#include "components/network_peer_component.h"

PreTickNetworkSystem::PreTickNetworkSystem()
    : ECS::ISimpleSystem()
{
}

static void Server_SpawnRemotePeerConnect( ECS::EntityContainer& entityContainer, uint32 remotePeerId )
{
	// Spawn its local player entity:
	GameEntity networkPeerEntity = entityContainer.GetFirstEntityOfType< NetworkPeerComponent >();
	NetworkPeerComponent& networkPeerComponent = networkPeerEntity.GetComponent< NetworkPeerComponent >();
	NetLib::Server* serverPeer = networkPeerComponent.GetPeerAsServer();
	serverPeer->CreateNetworkEntity( 10, remotePeerId, 0.f, 0.f );
}

void PreTickNetworkSystem::Execute( std::vector< GameEntity >& entities, ECS::EntityContainer& entity_container,
                                    float32 elapsed_time )
{
	for ( auto it = entities.begin(); it != entities.end(); ++it )
	{
		NetworkPeerComponent& networkPeerComponent = it->GetComponent< NetworkPeerComponent >();

		if ( networkPeerComponent.peer->GetConnectionState() == NetLib::PCS_Disconnected )
		{
			networkPeerComponent.peer->Start();
		}
		else
		{
			networkPeerComponent.peer->PreTick();
		}

		// Process new remote peer connections
		while ( !networkPeerComponent.unprocessedConnectedRemotePeers.empty() )
		{
			uint32 unprocessedConnectedRemotePeerId = networkPeerComponent.unprocessedConnectedRemotePeers.front();
			networkPeerComponent.unprocessedConnectedRemotePeers.pop();
			Server_SpawnRemotePeerConnect( entity_container, unprocessedConnectedRemotePeerId );
		}
	}
}
