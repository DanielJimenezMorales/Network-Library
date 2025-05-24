#include "crosshair_follow_mouse_system.h"

#include "vec2f.h"

#include "ecs/game_entity.hpp"
#include "ecs/world.h"

#include "components/virtual_mouse_component.h"
#include "components/crosshair_component.h"
#include "components/transform_component.h"

void CrosshairFollowMouseSystem::Execute( Engine::ECS::World& world, float32 elapsed_time )
{
	const Engine::ECS::GameEntity& virtual_mouse_entity = world.GetFirstEntityOfType< VirtualMouseComponent >();

	const Engine::TransformComponent& transform = virtual_mouse_entity.GetComponent< Engine::TransformComponent >();

	Engine::ECS::GameEntity entity = world.GetFirstEntityOfType< CrosshairComponent >();

	// Update crosshair world position
	Engine::TransformComponent& crosshairTransformComponent = entity.GetComponent< Engine::TransformComponent >();
	crosshairTransformComponent.SetPosition( transform.GetPosition() );
}
