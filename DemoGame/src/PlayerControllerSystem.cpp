#include "PlayerControllerSystem.h"
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
#include "Client.h"
#include <vector>

void PlayerControllerSystem::Tick(EntityContainer& entityContainer, float elapsedTime) const
{
	InputState inputState;
	ProcessInputs(entityContainer, inputState);
	//SendInputsToServer(entityContainer, inputState);

	std::vector<GameEntity> playerEntities = entityContainer.GetEntitiesOfType<PlayerControllerComponent>();
	auto it = playerEntities.begin();
	for (; it != playerEntities.end(); ++it)
	{
		TickPlayerController(*it, inputState, elapsedTime);
	}
}

void PlayerControllerSystem::ProcessInputs(EntityContainer& entityContainer, InputState& outInputState) const
{
	const GameEntity inputEntity = entityContainer.GetFirstEntityOfType<InputComponent>();
	const InputComponent& inputComponent = inputEntity.GetComponent<InputComponent>();

	outInputState.movement.X(inputComponent.inputController->GetAxis(HORIZONTAL_AXIS));
	outInputState.movement.Y(inputComponent.inputController->GetAxis(VERTICAL_AXIS));
	outInputState.movement.Normalize();
}

void PlayerControllerSystem::SendInputsToServer(EntityContainer& entityContainer, const InputState& inputState) const
{
	GameEntity networkPeerEntity = entityContainer.GetFirstEntityOfType<NetworkPeerComponent>();
	NetworkPeerComponent& networkPeerComponent = networkPeerEntity.GetComponent<NetworkPeerComponent>();

	NetLib::Client& networkClient = *static_cast<NetLib::Client*>(networkPeerComponent.peer);
	networkClient.SendInputs(inputState);
}

void PlayerControllerSystem::TickPlayerController(GameEntity& playerEntity, const InputState& inputState, float elapsedTime) const
{
	TransformComponent& transform = playerEntity.GetComponent<TransformComponent>();

	PlayerControllerComponent& networkComponent = playerEntity.GetComponent<PlayerControllerComponent>();
	Vec2f updatedPosition = UpdatePosition(inputState.movement, transform, networkComponent.configuration, elapsedTime);
	ApplyPosition(updatedPosition, transform);

	networkComponent.posX = updatedPosition.X();
	networkComponent.posY = updatedPosition.Y();
}

Vec2f PlayerControllerSystem::UpdatePosition(const Vec2f& inputs, const TransformComponent& transform, const PlayerControllerConfiguration& configuration, float elapsedTime) const
{
	Vec2f currentPosition = transform.position;

	currentPosition.AddToX(inputs.X() * configuration.movementSpeed * elapsedTime);
	currentPosition.AddToY(inputs.Y() * configuration.movementSpeed * elapsedTime);

	return currentPosition;
}

void PlayerControllerSystem::ApplyPosition(const Vec2f& position, TransformComponent& transform) const
{
	transform.position = position;
}
