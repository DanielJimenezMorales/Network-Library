#include "PlayerControllerSystem.h"
#include "EntityContainer.h"
#include "GameEntity.hpp"
#include "InputComponent.h"
#include "IInputController.h"
#include "Vec2f.h"
#include "TransformComponent.h"
#include "PlayerControllerComponent.h"
#include "InputActionIdsConfiguration.h"
#include <vector>

void PlayerControllerSystem::Tick(EntityContainer& entityContainer, float elapsedTime) const
{
	std::vector<GameEntity> playerEntities = entityContainer.GetEntitiesOfType<PlayerControllerComponent>();
	auto it = playerEntities.begin();
	for (; it != playerEntities.end(); ++it)
	{
		TickPlayerController(*it, elapsedTime);
	}
}

void PlayerControllerSystem::TickPlayerController(GameEntity& playerEntity, float elapsedTime) const
{
	const InputComponent& inputComponent = playerEntity.GetComponent<InputComponent>();
	Vec2f inputs(inputComponent.inputController->GetAxis(HORIZONTAL_AXIS), inputComponent.inputController->GetAxis(VERTICAL_AXIS));
	inputs.Normalize();

	TransformComponent& transform = playerEntity.GetComponent<TransformComponent>();

	Vec2f updatedPosition = UpdatePosition(inputs, transform, elapsedTime);
	ApplyPosition(updatedPosition, transform);

	PlayerControllerComponent& networkComponent = playerEntity.GetComponent<PlayerControllerComponent>();
	networkComponent.posX = updatedPosition.X();
	networkComponent.posY = updatedPosition.Y();
}

Vec2f PlayerControllerSystem::UpdatePosition(const Vec2f& inputs, const TransformComponent& transform, float elapsedTime) const
{
	//temp
	const PlayerControllerConfiguration config;

	Vec2f currentPosition = transform.position;

	currentPosition.AddToX(inputs.X() * config.movementSpeed * elapsedTime);
	currentPosition.AddToY(inputs.Y() * config.movementSpeed * elapsedTime);

	return currentPosition;
}

void PlayerControllerSystem::ApplyPosition(const Vec2f& position, TransformComponent& transform) const
{
	transform.position = position;
}
