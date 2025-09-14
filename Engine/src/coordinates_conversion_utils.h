#pragma once
#include "vec2f.h"

namespace Engine
{
	namespace ECS
	{
		class GameEntity;
	}

	struct CameraComponent;
	class ReadOnlyTransformComponentProxy;

	Vec2f ConvertFromWorldPositionToScreenPosition( Vec2f worldPosition, const ECS::GameEntity& camera_entity );
	Vec2f ConvertFromWorldPositionToScreenPosition( Vec2f worldPosition, const CameraComponent& camera,
	                                                ReadOnlyTransformComponentProxy& camera_transform );
	Vec2f ConvertFromScreenPositionToWorldPosition( Vec2f screenPosition, const ECS::GameEntity& camera_entity );
	Vec2f ConvertFromScreenPositionToWorldPosition( Vec2f screenPosition, const CameraComponent& camera,
	                                                ReadOnlyTransformComponentProxy& camera_transform );
}
