#pragma once
#include "numeric_types.h"

#include "ecs/game_entity.hpp"

#include "vec2f.h"

namespace Engine
{
	struct TransformComponent;

	/// <summary>
	/// This class provides an interface to systems for consuming the Engine::TransformComponent struct. This is the
	/// only way of consuming the Transform Component. The purpose of this proxy is to control the correct behaviour of
	/// the parent-children hierarchy.
	/// </summary>
	class ReadOnlyTransformComponentProxy
	{
		public:
			ReadOnlyTransformComponentProxy( const ECS::GameEntity& entity );

			// Position
			Vec2f GetGlobalPosition();

			// Rotation
			float32 GetGlobalRotationAngle();
			Vec2f GetForwardVector() const;

			// Scale
			Vec2f GetGlobalScale();

		private:
			const ECS::GameEntity _entity;
			const TransformComponent* _transformComponent;
	};
} // namespace Engine
