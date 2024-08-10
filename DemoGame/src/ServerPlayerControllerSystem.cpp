#include "ServerPlayerControllerSystem.h"
#include "EntityContainer.h"
#include "GameEntity.hpp"
#include "InputComponent.h"
#include "IInputController.h"
#include "Vec2f.h"
#include "TransformComponent.h"
#include "PlayerControllerComponent.h"
#include "InputActionIdsConfiguration.h"
#include "InputState.h"
#include "NetworkPeerComponent.h"
#include "NetworkEntityComponent.h"
#include "Client.h"
#include <vector>

void ServerPlayerControllerSystem::Tick(EntityContainer& entityContainer, float elapsedTime) const
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
		TickPlayerController(*it, *inputState, elapsedTime);
	}
}

void ServerPlayerControllerSystem::TickPlayerController(GameEntity& playerEntity, const InputState& inputState, float elapsedTime) const
{
	TransformComponent& transform = playerEntity.GetComponent<TransformComponent>();
	if (inputState.movement.X() != 0 || inputState.movement.Y() != 0)
	{
		LOG_WARNING("MOVEMENT {x: %f, y: %f}", inputState.movement.X(), inputState.movement.Y());
	}
	PlayerControllerComponent& networkComponent = playerEntity.GetComponent<PlayerControllerComponent>();
	Vec2f updatedPosition = UpdatePosition(inputState.movement, transform, networkComponent.configuration, elapsedTime);
	ApplyPosition(updatedPosition, transform);

	networkComponent.posX = updatedPosition.X();
	networkComponent.posY = updatedPosition.Y();

	Vec2f updatedLookAt = UpdateLookAt(inputState.mouseDeltaX, inputState.mouseDeltaY, transform, networkComponent.configuration, elapsedTime);
}

Vec2f ServerPlayerControllerSystem::UpdatePosition(const Vec2f& inputs, const TransformComponent& transform, const PlayerControllerConfiguration& configuration, float elapsedTime) const
{
	Vec2f currentPosition = transform.GetPosition();

	currentPosition.AddToX(inputs.X() * configuration.movementSpeed * elapsedTime);
	currentPosition.AddToY(inputs.Y() * configuration.movementSpeed * elapsedTime);

	return currentPosition;
}

void ServerPlayerControllerSystem::ApplyPosition(const Vec2f& position, TransformComponent& transform) const
{
	transform.SetPosition(position);
}

Vec2f ServerPlayerControllerSystem::UpdateLookAt(int32_t mouseDeltaX, int32_t mouseDeltaY, const TransformComponent& transform, const PlayerControllerConfiguration& configuration, float elapsedTime) const
{

	return Vec2f();
}
