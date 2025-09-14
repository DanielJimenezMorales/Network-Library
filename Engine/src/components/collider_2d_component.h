#pragma once
#include "vec2f.h"

#include "collisions/bounds_2d.h"

#include <cstdint>
#include <vector>
#include <memory>
#include <string>

namespace Engine
{
	class ReadOnlyTransformComponentProxy;

	enum class CollisionResponseType : uint8
	{
		None = 0,
		Static = 1,
		Dynamic = 2
	};

	CollisionResponseType GetCollisionResponseTypeFromName( const std::string& name );

	struct Collider2DComponent
	{
		public:
			Collider2DComponent();
			Collider2DComponent( Bounds2D* bounds, bool isTrigger, CollisionResponseType responseType );
			Collider2DComponent( const Collider2DComponent& ) = delete;
			Collider2DComponent( Collider2DComponent&& other ) noexcept;

			~Collider2DComponent();

			Collider2DComponent& operator=( const Collider2DComponent& ) = delete;
			Collider2DComponent& operator=( Collider2DComponent&& other ) noexcept;

			const Bounds2D* GetBounds2D() const { return _bounds; }
			CollisionShapeType GetShapeType() const { return _bounds->GetShapeType(); }
			void GetAxes( ReadOnlyTransformComponentProxy& transform, std::vector< Vec2f >& outAxes ) const;
			void ProjectAxis( ReadOnlyTransformComponentProxy& transform, const Vec2f& axis, float& outMin,
			                  float& outMax ) const;
			Vec2f GetClosestVertex( ReadOnlyTransformComponentProxy& transform, const Vec2f& inputPoint ) const;

			float32 GetMinX( ReadOnlyTransformComponentProxy& transform ) const;
			float32 GetMaxX( ReadOnlyTransformComponentProxy& transform ) const;

			std::unique_ptr< GizmoConfiguration > GetGizmo() const;

			CollisionResponseType GetCollisionResponse() const { return _collisionResponseType; }
			void SetCollisionResponse( CollisionResponseType collision_response_type )
			{
				_collisionResponseType = collision_response_type;
			}
			bool IsTrigger() const { return _isTrigger; }
			void SetIsTrigger( bool is_trigger ) { _isTrigger = is_trigger; }

			void SetBounds( Bounds2D* bounds ) { _bounds = bounds; }

		private:
			Bounds2D* _bounds;
			bool _isTrigger;
			CollisionResponseType _collisionResponseType;
	};
} // namespace Engine
