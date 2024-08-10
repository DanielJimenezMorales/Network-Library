#include "CrosshairFollowMouseSystem.h"
#include "GameEntity.hpp"
#include "CameraComponent.h"
#include "TransformComponent.h"
#include "CrosshairComponent.h"
#include "InputComponent.h"
#include "ICursor.h"
#include "Vec2f.h"

void CrosshairFollowMouseSystem::Update(EntityContainer& entityContainer, float elapsedTime) const
{
	const GameEntity cameraEntity = entityContainer.GetFirstEntityOfType<CameraComponent>();
	const CameraComponent& cameraComponent = cameraEntity.GetComponent<CameraComponent>();

	const GameEntity inputEntity = entityContainer.GetFirstEntityOfType<InputComponent>();
	const InputComponent& inputComponent = inputEntity.GetComponent<InputComponent>();

	const ICursor* activeCursor = inputComponent.cursor;

	//Calculate mouse world position
	int mouseScreenPositionX, mouseScreenPositionY = 0;
	activeCursor->GetPosition(mouseScreenPositionX, mouseScreenPositionY);

	Vec2f mouseScreenPosition(static_cast<float>(mouseScreenPositionX), static_cast<float>(mouseScreenPositionY));
	Vec2f mouseWorldPosition = cameraComponent.ConvertFromScreenPositionToWorldPosition(mouseScreenPosition);

	//Update crosshair world position
	GameEntity crosshairEntity = entityContainer.GetFirstEntityOfType<CrosshairComponent>();
	TransformComponent& crosshairTransformComponent = crosshairEntity.GetComponent<TransformComponent>();
	crosshairTransformComponent.SetPosition(mouseWorldPosition);
}
