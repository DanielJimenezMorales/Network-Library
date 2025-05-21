#include "virtual_mouse_system.h"

#include "inputs/ICursor.h"

#include "ecs/game_entity.hpp"
#include "ecs/world.h"

#include "components/virtual_mouse_component.h"
#include "components/camera_component.h"
#include "components/input_component.h"
#include "components/transform_component.h"

void VirtualMouseSystem::Execute( ECS::World& world, float32 elapsed_time )
{
	const CameraComponent& cameraComponent = world.GetFirstComponentOfType< CameraComponent >();
	const InputComponent& inputComponent = world.GetGlobalComponent< InputComponent >();

	std::vector< ECS::GameEntity > entities = world.GetEntitiesOfType< VirtualMouseComponent >();
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
