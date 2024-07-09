#include "NetworkSystem.h"
#include "Client.h"
#include "Server.h"
#include "Logger.h"
#include "EntityContainer.h"
#include "GameEntity.hpp"
#include "NetworkPeerComponent.h"
#include <vector>

void NetworkSystem::PreTick(EntityContainer& entityContainer, float elapsedTime) const
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
		uint32_t unprocessedConnectedRemotePeerId = networkPeerComponent.unprocessedConnectedRemotePeers.front();
		networkPeerComponent.unprocessedConnectedRemotePeers.pop();
		Server_SpawnRemotePeerConnect(entityContainer, unprocessedConnectedRemotePeerId);
	}
}

void NetworkSystem::PosTick(EntityContainer& entityContainer, float elapsedTime) const
{
	GameEntity networkPeerEntity = entityContainer.GetFirstEntityOfType<NetworkPeerComponent>();
	NetworkPeerComponent& networkPeerComponent = networkPeerEntity.GetComponent<NetworkPeerComponent>();
	networkPeerComponent.peer->Tick(elapsedTime);
}

void NetworkSystem::Server_SpawnRemotePeerConnect(EntityContainer& entityContainer, uint32_t remotePeerId) const
{
	//Spawn its local player entity:
	GameEntity networkPeerEntity = entityContainer.GetFirstEntityOfType<NetworkPeerComponent>();
	NetworkPeerComponent& networkPeerComponent = networkPeerEntity.GetComponent<NetworkPeerComponent>();
	NetLib::Server* serverPeer = networkPeerComponent.GetPeerAsServer();
	serverPeer->CreateNetworkEntity(10, remotePeerId, 256.f, 256.f);
}
