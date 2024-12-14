#include "NetworkSystem.h"
#include "core/client.h"
#include "core/server.h"
#include "Logger.h"
#include "EntityContainer.h"
#include "GameEntity.hpp"
#include "NetworkPeerComponent.h"
#include <vector>

void NetworkSystem::PreTick(EntityContainer& entityContainer, float32 elapsedTime) const
{
	GameEntity networkPeerEntity = entityContainer.GetFirstEntityOfType<NetworkPeerComponent>();
	NetworkPeerComponent& networkPeerComponent = networkPeerEntity.GetComponent<NetworkPeerComponent>();

	if (networkPeerComponent.peer->GetConnectionState() == NetLib::PCS_Disconnected)
	{
		networkPeerComponent.peer->Start();
	}
	else
	{
		networkPeerComponent.peer->PreTick();
	}

	//Process new remote peer connections
	while (!networkPeerComponent.unprocessedConnectedRemotePeers.empty())
	{
		uint32 unprocessedConnectedRemotePeerId = networkPeerComponent.unprocessedConnectedRemotePeers.front();
		networkPeerComponent.unprocessedConnectedRemotePeers.pop();
		Server_SpawnRemotePeerConnect(entityContainer, unprocessedConnectedRemotePeerId);
	}
}

void NetworkSystem::PosTick(EntityContainer& entityContainer, float32 elapsedTime) const
{
	GameEntity networkPeerEntity = entityContainer.GetFirstEntityOfType<NetworkPeerComponent>();
	NetworkPeerComponent& networkPeerComponent = networkPeerEntity.GetComponent<NetworkPeerComponent>();
	networkPeerComponent.peer->Tick(elapsedTime);
}

void NetworkSystem::Server_SpawnRemotePeerConnect(EntityContainer& entityContainer, uint32 remotePeerId) const
{
	//Spawn its local player entity:
	GameEntity networkPeerEntity = entityContainer.GetFirstEntityOfType<NetworkPeerComponent>();
	NetworkPeerComponent& networkPeerComponent = networkPeerEntity.GetComponent<NetworkPeerComponent>();
	NetLib::Server* serverPeer = networkPeerComponent.GetPeerAsServer();
	serverPeer->CreateNetworkEntity(10, remotePeerId, 0.f, 0.f);
}
