#include "CrosshairFollowMouseSystem.h"
#include "GameEntity.hpp"
#include "TransformComponent.h"
#include "CrosshairComponent.h"
#include "InputComponent.h"
#include "Vec2f.h"
#include "VirtualMouseComponent.h"

void CrosshairFollowMouseSystem::Update( ECS::EntityContainer& entityContainer, float32 elapsedTime ) const
{
	VirtualMouseComponent& virtualMouseComponent = entityContainer.GetFirstComponentOfType< VirtualMouseComponent >();

	// Update crosshair world position
	GameEntity crosshairEntity = entityContainer.GetFirstEntityOfType< CrosshairComponent >();
	TransformComponent& crosshairTransformComponent = crosshairEntity.GetComponent< TransformComponent >();
	crosshairTransformComponent.SetPosition( virtualMouseComponent.position );
}
