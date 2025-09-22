#pragma once
#include "numeric_types.h"

#include "ecs/game_entity.hpp"

#include "read_only_transform_component_proxy.h"

#include "vec2f.h"

namespace Engine
{
	struct TransformComponent;

	/// <summary>
	/// This class provides an interface to systems for consuming the Engine::TransformComponent struct. This is the
	/// only way of consuming the Transform Component. The purpose of this proxy is to control the correct behaviour of
	/// the parent-children hierarchy.
	/// </summary>
	class TransformComponentProxy
	{
		public:
			TransformComponentProxy( ECS::GameEntity& entity );
			ReadOnlyTransformComponentProxy AsReadOnly() const { return ReadOnlyTransformComponentProxy( _entity ); }

			// Position
			Vec2f GetGlobalPosition();
			void SetGlobalPosition( const Vec2f& new_position );

			// Rotation
			float32 GetGlobalRotation();
			void SetGlobalRotationAngle( float32 new_angle );
			void SetRotationLookAt( Vec2f look_at_direction );
			void LookAt( const Vec2f& position );
			Vec2f GetForwardVector();

			// Scale
			Vec2f GetGlobalScale();
			void SetGlobalScale( const Vec2f& new_scale );

			// Parent-Child relationships
			void RemoveParent();
			void SetParent( ECS::GameEntity& parent_entity );
			bool HasParent() const;
			bool HasChildren() const;
			const std::vector< ECS::GameEntity >& GetChildren() const;
			std::vector< ECS::GameEntity > GetChildren();

		private:
			void SetChildrenDirty();
			void ResolveDirty();

			ECS::GameEntity _entity;
			TransformComponent* _transformComponent;
	};
} // namespace Engine
