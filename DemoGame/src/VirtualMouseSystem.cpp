#include "VirtualMouseSystem.h"
#include "EntityContainer.h"
#include "VirtualMouseComponent.h"
#include "GameEntity.hpp"
#include "CameraComponent.h"

void VirtualMouseSystem::Update(EntityContainer& entityContainer, float elapsedTime) const
{
	ComponentView<VirtualMouseComponent> virtualMouseComponents = entityContainer.GetComponentsOfType<VirtualMouseComponent>();

	const CameraComponent& cameraComponent = entityContainer.GetFirstComponentOfType<CameraComponent>();

	while (virtualMouseComponents.ArePendingComponents())
	{
		VirtualMouseComponent& virtualMouseComponent = virtualMouseComponents.GetNext();
	}
}
