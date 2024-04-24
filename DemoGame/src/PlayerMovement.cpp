#include "PlayerMovement.h"
#include "InputComponent.h"
#include "InputActionIdsConfiguration.h"

void PlayerMovement::Tick(float tickElapsedTime)
{
	InputComponent& inputComponent = GetComponent<InputComponent>();
	Vec2f inputs(inputComponent.inputController->GetAxis(HORIZONTAL_AXIS), inputComponent.inputController->GetAxis(VERTICAL_AXIS));
	inputs.Normalize();

	TransformComponent& transform = GetComponent<TransformComponent>();

	Vec2f updatedPosition = UpdatePosition(inputs, transform, tickElapsedTime);
	ApplyPosition(updatedPosition, transform);
}

Vec2f PlayerMovement::UpdatePosition(const Vec2f& inputs, const TransformComponent& transform, float elapsedTime) const
{
	float currentXPosition = transform.posX;
	float currentYPosition = transform.posY;

	currentXPosition += inputs.X() * _speed * elapsedTime;
	currentYPosition += inputs.Y() * _speed * elapsedTime;

	return Vec2f(currentXPosition, currentYPosition);
}

void PlayerMovement::ApplyPosition(const Vec2f& position, TransformComponent& transform)
{
	transform.posX = position.X();
	transform.posY = position.Y();
}
