#include "pre_tick_network_system.h"

#include "ecs/game_entity.hpp"
#include "ecs/world.h"

#include "global_components/network_peer_global_component.h"

PreTickNetworkSystem::PreTickNetworkSystem()
    : ECS::ISimpleSystem()
{
}

static void Server_SpawnRemotePeerConnect( ECS::World& world, uint32 remotePeerId )
{
	// Spawn its local player entity:
	NetworkPeerGlobalComponent& networkPeerComponent = world.GetGlobalComponent< NetworkPeerGlobalComponent >();
	NetLib::Server* serverPeer = networkPeerComponent.GetPeerAsServer();
	serverPeer->CreateNetworkEntity( 10, remotePeerId, 0.f, 0.f );
}

void PreTickNetworkSystem::Execute( ECS::World& world, float32 elapsed_time )
{
	NetworkPeerGlobalComponent& networkPeerComponent = world.GetGlobalComponent< NetworkPeerGlobalComponent >();

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
		Server_SpawnRemotePeerConnect( world, unprocessedConnectedRemotePeerId );
	}
}
