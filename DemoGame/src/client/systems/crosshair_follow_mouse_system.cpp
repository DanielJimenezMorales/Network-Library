#include "crosshair_follow_mouse_system.h"

#include "vec2f.h"

#include "ecs/game_entity.hpp"
#include "ecs/world.h"

#include "components/transform_component.h"

#include "transform/transform_hierarchy_helper_functions.h"

#include "client/components/virtual_mouse_component.h"
#include "client/components/crosshair_component.h"

void CrosshairFollowMouseSystem::Execute( Engine::ECS::World& world, float32 elapsed_time )
{
	const Engine::TransformComponentProxy transformComponentProxy;

	const Engine::ECS::GameEntity& virtual_mouse_entity = world.GetFirstEntityOfType< VirtualMouseComponent >();

	const Engine::TransformComponent& virtualMousetransform =
	    virtual_mouse_entity.GetComponent< Engine::TransformComponent >();
	Engine::ECS::GameEntity entity = world.GetFirstEntityOfType< CrosshairComponent >();

	// Update crosshair world position
	Engine::TransformComponent& crosshairTransform = entity.GetComponent< Engine::TransformComponent >();
	transformComponentProxy.SetGlobalPosition( crosshairTransform,
	                                           transformComponentProxy.GetGlobalPosition( virtualMousetransform ) );
}
