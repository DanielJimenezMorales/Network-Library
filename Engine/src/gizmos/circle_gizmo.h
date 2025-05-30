#pragma once
#include "gizmos/Gizmo.h"

namespace Engine
{
	struct CircleGizmoConfiguration : public GizmoConfiguration
	{
			float radius;
	};

	class CircleGizmo : public Gizmo
	{
		public:
			CircleGizmo()
			    : Gizmo( 0, 0, 0, 255, GizmoType::CIRCLE2D )
			    , _radius( 0 )
			{
			}

			CircleGizmo( float32 radius )
			    : Gizmo( 0, 255, 0, 255, GizmoType::CIRCLE2D )
			    , _radius( radius )
			{
			}

			CircleGizmo( const CircleGizmoConfiguration* configuration )
			    : Gizmo( configuration )
			    , _radius( configuration->radius )
			{
			}

			CircleGizmo( const CircleGizmo& other ) = default;
			CircleGizmo( CircleGizmo&& other ) noexcept = default;

			CircleGizmo& operator=( const CircleGizmo& other ) = default;
			CircleGizmo& operator=( CircleGizmo&& other ) noexcept = default;

			float32 GetRadius() const { return _radius; }

			CircleGizmo* Clone() const override;

		protected:
			void ConfigureConcrete( const GizmoConfiguration* configuration ) override;

		private:
			float32 _radius;
	};
} // namespace Engine
