#include "pre_tick_network_system.h"

#include "ecs/game_entity.hpp"
#include "ecs/world.h"

#include "shared/global_components/network_peer_global_component.h"
#include "shared/networked_entity_types.h"

PreTickNetworkSystem::PreTickNetworkSystem( const std::string& ip, uint32 port )
    : Engine::ECS::ISimpleSystem()
    , _ip( ip )
    , _port( port )
{
}

static void Server_SpawnRemotePeerConnect( Engine::ECS::World& world, uint32 remotePeerId )
{
	// Spawn its local player entity:
	NetworkPeerGlobalComponent& networkPeerComponent = world.GetGlobalComponent< NetworkPeerGlobalComponent >();
	NetLib::Server* serverPeer = networkPeerComponent.GetPeerAsServer();
	serverPeer->CreateNetworkEntity( PLAYER_NETWORKED_ENTITY_TYPE, remotePeerId, 0.f, 0.f );
}

void PreTickNetworkSystem::Execute( Engine::ECS::World& world, float32 elapsed_time )
{
	NetworkPeerGlobalComponent& networkPeerComponent = world.GetGlobalComponent< NetworkPeerGlobalComponent >();

	if ( networkPeerComponent.peer->GetConnectionState() == NetLib::PCS_Disconnected )
	{
		networkPeerComponent.peer->Start( _ip, _port );
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
