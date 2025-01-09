#include "crosshair_follow_mouse_system.h"

#include "GameEntity.hpp"
#include "Vec2f.h"

#include "ecs/entity_container.h"

#include "components/virtual_mouse_component.h"
#include "components/crosshair_component.h"
#include "components/transform_component.h"

void CrosshairFollowMouseSystem::Execute( std::vector< GameEntity >& entities, ECS::EntityContainer& entity_container,
                                          float32 elapsed_time )
{
	const VirtualMouseComponent& virtual_mouse_component =
	    entity_container.GetFirstComponentOfType< VirtualMouseComponent >();

	for ( auto it = entities.begin(); it != entities.end(); ++it )
	{
		// Update crosshair world position
		TransformComponent& crosshairTransformComponent = it->GetComponent< TransformComponent >();
		crosshairTransformComponent.SetPosition( virtual_mouse_component.position );
	}
}
