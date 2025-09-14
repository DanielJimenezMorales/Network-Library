#include "virtual_mouse_system.h"

#include "inputs/i_cursor.h"

#include "ecs/game_entity.hpp"
#include "ecs/world.h"

#include "components/camera_component.h"

#include "transform_component_proxy.h"

#include "client/components/virtual_mouse_component.h"

#include "global_components/input_handler_global_component.h"

#include "shared/InputActionIdsConfiguration.h"

void VirtualMouseSystem::Execute( Engine::ECS::World& world, float32 elapsed_time )
{
	const Engine::CameraComponent& cameraComponent = world.GetFirstComponentOfType< Engine::CameraComponent >();
	const Engine::InputHandlerGlobalComponent& inputHandlerGlobalComponent =
	    world.GetGlobalComponent< Engine::InputHandlerGlobalComponent >();

	std::vector< Engine::ECS::GameEntity > entities = world.GetEntitiesOfType< VirtualMouseComponent >();
	for ( auto it = entities.begin(); it != entities.end(); ++it )
	{
		Engine::TransformComponentProxy virtualMouseTransform( *it );

		int32 mouseDeltaX, mouseDeltaY = 0;
		inputHandlerGlobalComponent.CursorGetDelta( MOUSE_NAME, mouseDeltaX, mouseDeltaY );

		// The minus Y here is because SDL tracks positive Y downwards
		Vec2f distance( mouseDeltaX, -mouseDeltaY );
		distance *= 100 * elapsed_time;

		const Vec2f new_position = virtualMouseTransform.GetGlobalPosition() + distance;
		virtualMouseTransform.SetGlobalPosition( new_position );
	}
}
