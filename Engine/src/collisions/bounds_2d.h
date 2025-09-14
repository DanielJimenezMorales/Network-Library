#pragma once
#include "vec2f.h"
#include "numeric_types.h"

#include "gizmos/gizmo.h"

#include <vector>
#include <memory>

namespace Engine
{
	class ReadOnlyTransformComponentProxy;

	enum class CollisionShapeType : uint8
	{
		Convex = 0,
		Circle = 1,
	};

	struct Bounds2DConfiguration
	{
			virtual ~Bounds2DConfiguration() {}

			virtual Bounds2DConfiguration* Clone() const = 0;

			CollisionShapeType type;

		protected:
			Bounds2DConfiguration( CollisionShapeType type )
			    : type( type )
			{
			}
	};

	class Bounds2D
	{
		public:
			virtual ~Bounds2D() {}

			CollisionShapeType GetShapeType() const { return _shapeType; }
			virtual void GetAxes( ReadOnlyTransformComponentProxy& transform, std::vector< Vec2f >& outAxes ) const = 0;
			virtual void ProjectAxis( ReadOnlyTransformComponentProxy& transform, const Vec2f& axis, float& outMin,
			                          float& outMax ) const = 0;
			virtual Vec2f GetClosestVertex( ReadOnlyTransformComponentProxy& transform,
			                                const Vec2f& inputPoint ) const = 0;

			virtual float32 GetMinX( ReadOnlyTransformComponentProxy& transform ) const = 0;
			virtual float32 GetMaxX( ReadOnlyTransformComponentProxy& transform ) const = 0;

			// Separate this one in a callback where it adds a GizmoComponent everytime an entity with a collider is
			// spawned and the global variable ENABLE_GIZMOS is enabled
			virtual std::unique_ptr< GizmoConfiguration > GetGizmo() const = 0;

		protected:
			Bounds2D( CollisionShapeType shapeType )
			    : _shapeType( shapeType )
			{
			}

		private:
			CollisionShapeType _shapeType;
	};
} // namespace Engine
