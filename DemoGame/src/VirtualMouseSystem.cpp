#include "VirtualMouseSystem.h"
#include "EntityContainer.h"
#include "VirtualMouseComponent.h"
#include "GameEntity.hpp"
#include "CameraComponent.h"
#include "InputComponent.h"
#include "ICursor.h"

void VirtualMouseSystem::Update(EntityContainer& entityContainer, float elapsedTime) const
{
	ComponentView<VirtualMouseComponent> virtualMouseComponents = entityContainer.GetComponentsOfType<VirtualMouseComponent>();

	const CameraComponent& cameraComponent = entityContainer.GetFirstComponentOfType<CameraComponent>();
	const InputComponent& inputComponent = entityContainer.GetFirstComponentOfType<InputComponent>();

	int mouseDeltaX, mouseDeltaY = 0;
	inputComponent.cursor->GetDelta(mouseDeltaX, mouseDeltaY);

	//The minus Y here is because SDL tracks positive Y downwards
	Vec2f distance(mouseDeltaX, -mouseDeltaY);
	distance *= 100 * elapsedTime;

	while (virtualMouseComponents.ArePendingComponents())
	{
		VirtualMouseComponent& virtualMouseComponent = virtualMouseComponents.GetNext();
		virtualMouseComponent.position.AddToX(distance.X());
		virtualMouseComponent.position.AddToY(distance.Y());
	}
}
