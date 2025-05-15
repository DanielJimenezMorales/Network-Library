#include "crosshair_follow_mouse_system.h"

#include "Vec2f.h"

#include "ecs/game_entity.hpp"
#include "ecs/world.h"

#include "components/virtual_mouse_component.h"
#include "components/crosshair_component.h"
#include "components/transform_component.h"

void CrosshairFollowMouseSystem::Execute( ECS::World& world, float32 elapsed_time )
{
	const ECS::GameEntity& virtual_mouse_entity = world.GetFirstEntityOfType< VirtualMouseComponent >();

	const TransformComponent& transform = virtual_mouse_entity.GetComponent< TransformComponent >();

	ECS::GameEntity entity = world.GetFirstEntityOfType< CrosshairComponent >();

	// Update crosshair world position
	TransformComponent& crosshairTransformComponent = entity.GetComponent< TransformComponent >();
	crosshairTransformComponent.SetPosition( transform.GetPosition() );
}
