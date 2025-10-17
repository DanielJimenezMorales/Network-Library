#pragma once
#include "numeric_types.h"

#include "Vec2f.h"
#include "ecs/game_entity.hpp"

#include <vector>
namespace Engine
{
	struct TransformComponent;

	class TransformComponentProxy
	{
		public:
			TransformComponentProxy() = default;

			// Position
			Vec2f GetGlobalPosition( const TransformComponent& transform ) const;
			void SetGlobalPosition( TransformComponent& transform, const Vec2f& new_position ) const;
			Vec2f GetLocalPosition( const TransformComponent& transform ) const;
			void SetLocalPosition( TransformComponent& transform, const Vec2f& new_local_position ) const;

			// Rotation
			float32 GetGlobalRotation( const TransformComponent& transform ) const;
			void SetGlobalRotationAngle( TransformComponent& transform, float32 new_angle ) const;
			float32 GetLocalRotationAngle( const TransformComponent& transform ) const;
			void SetLocalRotationAngle( TransformComponent& transform, float32 new_local_angle ) const;
			void SetRotationLookAt( TransformComponent& transform, Vec2f look_at_direction ) const;
			void LookAt( TransformComponent& transform, const Vec2f& position ) const;
			Vec2f GetForwardVector( const TransformComponent& transform ) const;

			// Scale
			Vec2f GetGlobalScale( const TransformComponent& transform ) const;
			void SetGlobalScale( TransformComponent& transform, const Vec2f& new_scale ) const;
			Vec2f GetLocalScale( const TransformComponent& transform ) const;
			void SetLocalScale( TransformComponent& transform, const Vec2f& new_local_scale ) const;

			// Parent-Child relationships
			void RemoveParent( TransformComponent& transform, ECS::GameEntity& transform_entity ) const;
			void SetParent( TransformComponent& transform, ECS::GameEntity& transform_entity,
			                ECS::GameEntity& parent_entity ) const;
			bool HasParent( const TransformComponent& transform ) const;
			ECS::GameEntity GetParent( const TransformComponent& transform ) const;
			bool HasChildren( const TransformComponent& transform ) const;
			std::vector< ECS::GameEntity > GetChildren( const TransformComponent& transform ) const;

		private:
			void SetChildrenDirty( const TransformComponent& transform ) const;
			void ResolveDirty( const TransformComponent& transform ) const;
			bool IsDirty( const TransformComponent& transform ) const;
	};
} // namespace Engine