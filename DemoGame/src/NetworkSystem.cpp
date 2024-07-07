#include "NetworkSystem.h"
#include "Client.h"
#include "Server.h"
#include "Logger.h"
#include "CurrentTickComponent.h"
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
}

void NetworkSystem::PosTick(EntityContainer& entityContainer, float elapsedTime) const
{
	GameEntity networkPeerEntity = entityContainer.GetFirstEntityOfType<NetworkPeerComponent>();
	NetworkPeerComponent& networkPeerComponent = networkPeerEntity.GetComponent<NetworkPeerComponent>();
	networkPeerComponent.peer->Tick(elapsedTime);

	GameEntity gameEntity = entityContainer.GetFirstEntityOfType<CurrentTickComponent>();
	CurrentTickComponent& currentTickComponent = gameEntity.GetComponent<CurrentTickComponent>();

	//TEMP
	if (networkPeerComponent.peer->GetPeerType() == NetLib::PeerType::ClientMode)
	{
		return;
	}
	if (currentTickComponent.currentTick == 10)
	{
		//TODO Create an key-value map to store different factories depending on the entity type
		//static_cast<NetLib::Server*>(networkPeerComponent.peer)->CreateNetworkEntity(10, 5, 256.f, 256.f);
	}
	++currentTickComponent.currentTick;
}
