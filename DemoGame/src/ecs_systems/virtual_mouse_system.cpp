#include "virtual_mouse_system.h"

#include "ICursor.h"

#include "ecs/game_entity.hpp"
#include "ecs/entity_container.h"

#include "components/virtual_mouse_component.h"
#include "components/camera_component.h"
#include "components/input_component.h"
#include "components/transform_component.h"

void VirtualMouseSystem::Execute( ECS::EntityContainer& entity_container, float32 elapsed_time )
{
	const CameraComponent& cameraComponent = entity_container.GetFirstComponentOfType< CameraComponent >();
	const InputComponent& inputComponent = entity_container.GetGlobalComponent< InputComponent >();

	std::vector< ECS::GameEntity > entities = entity_container.GetEntitiesOfType< VirtualMouseComponent >();
	for ( auto it = entities.begin(); it != entities.end(); ++it )
	{
		TransformComponent& virtual_mouse_transform = it->GetComponent< TransformComponent >();

		int32 mouseDeltaX, mouseDeltaY = 0;
		inputComponent.cursor->GetDelta( mouseDeltaX, mouseDeltaY );

		// The minus Y here is because SDL tracks positive Y downwards
		Vec2f distance( mouseDeltaX, -mouseDeltaY );
		distance *= 100 * elapsed_time;

		const Vec2f new_position = virtual_mouse_transform.GetPosition() + distance;
		virtual_mouse_transform.SetPosition( new_position );
	}
}
