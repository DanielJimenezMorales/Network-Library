#include "PlayerControllerSystem.h"
#include "EntityContainer.h"
#include "GameEntity.hpp"
#include "InputComponent.h"
#include "IInputController.h"
#include "ICursor.h"
#include "Vec2f.h"
#include "TransformComponent.h"
#include "PlayerControllerComponent.h"
#include "InputActionIdsConfiguration.h"
#include "InputState.h"
#include "NetworkPeerComponent.h"
#include "VirtualMouseComponent.h"

#include <vector>

#include "core/client.h"

void PlayerControllerSystem::Tick(EntityContainer& entityContainer, float32 elapsedTime) const
{
	std::vector<GameEntity> playerEntities = entityContainer.GetEntitiesOfType<PlayerControllerComponent>();
	if (playerEntities.empty())
	{
		return;
	}

	const GameEntity networkPeerEntity = entityContainer.GetFirstEntityOfType<NetworkPeerComponent>();
	const NetworkPeerComponent& networkPeerComponent = networkPeerEntity.GetComponent<NetworkPeerComponent>();
	if (networkPeerComponent.peer->GetConnectionState() != NetLib::PCS_Connected)
	{
		return;
	}

	InputState inputState;
	ProcessInputs(entityContainer, inputState);
	SendInputsToServer(entityContainer, inputState);

	auto it = playerEntities.begin();
	for (; it != playerEntities.end(); ++it)
	{
		TransformComponent& transform = it->GetComponent<TransformComponent>();
		const PlayerControllerComponent& networkComponent = it->GetComponent<PlayerControllerComponent>();

		transform.SetPosition(Vec2f(networkComponent.posX.Get(), networkComponent.posY.Get()));
		transform.SetRotationAngle(networkComponent.rotationAngle.Get());

		//TODO Enable this when client-side prediction and reconciliation is ready
		//TickPlayerController(*it, inputState, elapsedTime);
	}
}

void PlayerControllerSystem::ProcessInputs(EntityContainer& entityContainer, InputState& outInputState) const
{
	const InputComponent& inputComponent = entityContainer.GetFirstComponentOfType<InputComponent>();

	outInputState.movement.X(inputComponent.inputController->GetAxis(HORIZONTAL_AXIS));
	outInputState.movement.Y(inputComponent.inputController->GetAxis(VERTICAL_AXIS));
	outInputState.movement.Normalize();

	const VirtualMouseComponent& virtualMouseComponent = entityContainer.GetFirstComponentOfType<VirtualMouseComponent>();
	outInputState.virtualMousePosition = virtualMouseComponent.position;
}

void PlayerControllerSystem::SendInputsToServer(EntityContainer& entityContainer, const InputState& inputState) const
{
	GameEntity networkPeerEntity = entityContainer.GetFirstEntityOfType<NetworkPeerComponent>();
	NetworkPeerComponent& networkPeerComponent = networkPeerEntity.GetComponent<NetworkPeerComponent>();

	NetLib::Client& networkClient = *static_cast<NetLib::Client*>(networkPeerComponent.peer);
	networkClient.SendInputs(inputState);
}
