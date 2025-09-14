#include "crosshair_follow_mouse_system.h"

#include "vec2f.h"

#include "ecs/game_entity.hpp"
#include "ecs/world.h"

#include "transform_component_proxy.h"
#include "read_only_transform_component_proxy.h"

#include "client/components/virtual_mouse_component.h"
#include "client/components/crosshair_component.h"

void CrosshairFollowMouseSystem::Execute( Engine::ECS::World& world, float32 elapsed_time )
{
	const Engine::ECS::GameEntity& virtual_mouse_entity = world.GetFirstEntityOfType< VirtualMouseComponent >();

	Engine::ReadOnlyTransformComponentProxy virtualMousetransformComponentProxy( virtual_mouse_entity );
	Engine::ECS::GameEntity entity = world.GetFirstEntityOfType< CrosshairComponent >();

	// Update crosshair world position
	Engine::TransformComponentProxy crosshairTransformComponentProxy( entity );
	crosshairTransformComponentProxy.SetGlobalPosition( virtualMousetransformComponentProxy.GetGlobalPosition() );
}
