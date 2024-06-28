#include "PlayerMovement.h"
#include "InputComponent.h"
#include "IInputController.h"
#include "PlayerNetworkComponent.h"
#include "InputActionIdsConfiguration.h"
#include "TransformComponent.h"

void PlayerMovement::Tick(float tickElapsedTime)
{
	InputComponent& inputComponent = GetComponent<InputComponent>();
	Vec2f inputs(inputComponent.inputController->GetAxis(HORIZONTAL_AXIS), inputComponent.inputController->GetAxis(VERTICAL_AXIS));
	inputs.Normalize();

	TransformComponent& transform = GetComponent<TransformComponent>();

	Vec2f updatedPosition = UpdatePosition(inputs, transform, tickElapsedTime);
	ApplyPosition(updatedPosition, transform);

	PlayerNetworkComponent& networkComponent = GetComponent<PlayerNetworkComponent>();
	networkComponent.posX = updatedPosition.X();
	networkComponent.posY = updatedPosition.Y();
}

Vec2f PlayerMovement::UpdatePosition(const Vec2f& inputs, const TransformComponent& transform, float elapsedTime) const
{
	Vec2f currentPosition = transform.position;

	currentPosition.AddToX(inputs.X() * _speed * elapsedTime);
	currentPosition.AddToY(inputs.Y() * _speed * elapsedTime);

	return currentPosition;
}

void PlayerMovement::ApplyPosition(const Vec2f& position, TransformComponent& transform)
{
	transform.position = position;
}

void PlayerDummyMovement::Tick(float tickElapsedTime)
{
	TransformComponent& transform = GetComponent<TransformComponent>();
	PlayerNetworkComponent& networkComponent = GetComponent<PlayerNetworkComponent>();

	transform.position = Vec2f(networkComponent.posX.Get(), networkComponent.posY.Get());
}
