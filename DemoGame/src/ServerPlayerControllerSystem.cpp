#include "ServerPlayerControllerSystem.h"
#include "EntityContainer.h"
#include "GameEntity.hpp"
#include "PlayerControllerComponent.h"
#include "InputState.h"
#include "NetworkPeerComponent.h"
#include "NetworkEntityComponent.h"
#include "PlayerSimulator.h"
#include <vector>

void ServerPlayerControllerSystem::Tick(EntityContainer& entityContainer, float32 elapsedTime) const
{
	GameEntity& networkPeerEntity = entityContainer.GetFirstEntityOfType<NetworkPeerComponent>();
	NetworkPeerComponent& networkPeerComponent = networkPeerEntity.GetComponent<NetworkPeerComponent>();
	NetLib::Server* serverPeer = networkPeerComponent.GetPeerAsServer();

	std::vector<GameEntity> playerEntities = entityContainer.GetEntitiesOfType<PlayerControllerComponent>();
	auto it = playerEntities.begin();
	for (; it != playerEntities.end(); ++it)
	{
		const NetworkEntityComponent& networkEntityComponent = it->GetComponent<NetworkEntityComponent>();
		const NetLib::IInputState* baseInputState = serverPeer->GetInputFromRemotePeer(networkEntityComponent.controlledByPeerId);
		if (baseInputState == nullptr)
		{
			continue;
		}

		const InputState* inputState = static_cast<const InputState*>(baseInputState);
		PlayerSimulator::Simulate(*inputState, *it, elapsedTime);
	}
}
