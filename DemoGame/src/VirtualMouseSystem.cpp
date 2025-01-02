#include "VirtualMouseSystem.h"
#include "VirtualMouseComponent.h"
#include "GameEntity.hpp"
#include "CameraComponent.h"
#include "InputComponent.h"
#include "ICursor.h"

#include "ecs/entity_container.h"

void VirtualMouseSystem::Update( ECS::EntityContainer& entityContainer, float32 elapsedTime ) const
{
	ComponentView< VirtualMouseComponent > virtualMouseComponents =
	    entityContainer.GetComponentsOfType< VirtualMouseComponent >();

	const CameraComponent& cameraComponent = entityContainer.GetFirstComponentOfType< CameraComponent >();
	const InputComponent& inputComponent = entityContainer.GetFirstComponentOfType< InputComponent >();

	int32 mouseDeltaX, mouseDeltaY = 0;
	inputComponent.cursor->GetDelta( mouseDeltaX, mouseDeltaY );

	// The minus Y here is because SDL tracks positive Y downwards
	Vec2f distance( mouseDeltaX, -mouseDeltaY );
	distance *= 100 * elapsedTime;

	while ( virtualMouseComponents.ArePendingComponents() )
	{
		VirtualMouseComponent& virtualMouseComponent = virtualMouseComponents.GetNext();
		virtualMouseComponent.position.AddToX( distance.X() );
		virtualMouseComponent.position.AddToY( distance.Y() );
	}
}
