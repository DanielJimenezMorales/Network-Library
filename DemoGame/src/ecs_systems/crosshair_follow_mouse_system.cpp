#include "crosshair_follow_mouse_system.h"

#include "Vec2f.h"

#include "ecs/game_entity.hpp"
#include "ecs/entity_container.h"

#include "components/virtual_mouse_component.h"
#include "components/crosshair_component.h"
#include "components/transform_component.h"

void CrosshairFollowMouseSystem::Execute( ECS::EntityContainer& entity_container, float32 elapsed_time )
{
	const ECS::GameEntity& virtual_mouse_entity = entity_container.GetFirstEntityOfType< VirtualMouseComponent >();

	const TransformComponent& transform = virtual_mouse_entity.GetComponent< TransformComponent >();

	ECS::GameEntity entity = entity_container.GetFirstEntityOfType< CrosshairComponent >();

	// Update crosshair world position
	TransformComponent& crosshairTransformComponent = entity.GetComponent< TransformComponent >();
	crosshairTransformComponent.SetPosition( transform.GetPosition() );
}
