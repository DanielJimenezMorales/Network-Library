#include "crosshair_follow_mouse_system.h"

#include "GameEntity.hpp"
#include "TransformComponent.h"
#include "CrosshairComponent.h"
#include "Vec2f.h"
#include "VirtualMouseComponent.h"

#include "ecs/entity_container.h"

void CrosshairFollowMouseSystem::Execute( GameEntity& entity, float32 elapsed_time )
{
	const ECS::EntityContainer* entity_container = entity.GetEntityContainer();
	const VirtualMouseComponent& virtual_mouse_component =
	    entity_container->GetFirstComponentOfType< VirtualMouseComponent >();

	// Update crosshair world position
	TransformComponent& crosshairTransformComponent = entity.GetComponent< TransformComponent >();
	crosshairTransformComponent.SetPosition( virtual_mouse_component.position );
}
