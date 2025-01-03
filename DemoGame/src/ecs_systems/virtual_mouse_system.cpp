#include "virtual_mouse_system.h"

#include "GameEntity.hpp"
#include "ICursor.h"

#include "ecs/entity_container.h"

#include "components/virtual_mouse_component.h"
#include "components/camera_component.h"
#include "components/input_component.h"

void VirtualMouseSystem::Execute( GameEntity& entity, float32 elapsed_time )
{
	const ECS::EntityContainer* entity_container = entity.GetEntityContainer();

	const CameraComponent& cameraComponent = entity_container->GetFirstComponentOfType< CameraComponent >();
	const InputComponent& inputComponent = entity_container->GetFirstComponentOfType< InputComponent >();

	VirtualMouseComponent& virtual_mouse = entity.GetComponent< VirtualMouseComponent >();

	int32 mouseDeltaX, mouseDeltaY = 0;
	inputComponent.cursor->GetDelta( mouseDeltaX, mouseDeltaY );

	// The minus Y here is because SDL tracks positive Y downwards
	Vec2f distance( mouseDeltaX, -mouseDeltaY );
	distance *= 100 * elapsed_time;

	virtual_mouse.position.AddToX( distance.X() );
	virtual_mouse.position.AddToY( distance.Y() );
}
