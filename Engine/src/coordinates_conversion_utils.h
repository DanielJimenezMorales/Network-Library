#pragma once
#include "vec2f.h"

namespace Engine
{
	namespace ECS
	{
		class GameEntity;
	}

	struct CameraComponent;
	struct TransformComponent;

	Vec2f ConvertFromWorldPositionToScreenPosition( Vec2f worldPosition, const ECS::GameEntity& camera_entity );
	Vec2f ConvertFromWorldPositionToScreenPosition( Vec2f worldPosition, const CameraComponent& camera,
	                                                const Engine::TransformComponent& camera_transform );
	Vec2f ConvertFromScreenPositionToWorldPosition( Vec2f screenPosition, const ECS::GameEntity& camera_entity );
	Vec2f ConvertFromScreenPositionToWorldPosition( Vec2f screenPosition, const CameraComponent& camera,
	                                                const Engine::TransformComponent& camera_transform );
}
