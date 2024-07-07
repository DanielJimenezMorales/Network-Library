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
		SpawnRemotePeerConnect(entityContainer);
	}
}

void NetworkSystem::PosTick(EntityContainer& entityContainer, float elapsedTime) const
{
	GameEntity networkPeerEntity = entityContainer.GetFirstEntityOfType<NetworkPeerComponent>();
	NetworkPeerComponent& networkPeerComponent = networkPeerEntity.GetComponent<NetworkPeerComponent>();
	networkPeerComponent.peer->Tick(elapsedTime);
}

void NetworkSystem::SpawnRemotePeerConnect(EntityContainer& entityContainer) const
{
	GameEntity networkPeerEntity = entityContainer.GetFirstEntityOfType<NetworkPeerComponent>();
	NetworkPeerComponent& networkPeerComponent = networkPeerEntity.GetComponent<NetworkPeerComponent>();
	NetLib::Server* serverPeer = networkPeerComponent.GetPeerAsServer();
	serverPeer->CreateNetworkEntity(10, 5, 256.f, 256.f);
}
