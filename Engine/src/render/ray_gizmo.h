#pragma once
#include "render/gizmo.h"
#include "render/color.h"

namespace Engine
{
	struct RayGizmoConfiguration : public GizmoConfiguration
	{
			float length;
	};

	class RayGizmo : public Gizmo
	{
		public:
			RayGizmo()
			    : Gizmo( Color::WHITE(), GizmoType::RAY )
			    , _length( 0 )
			{
			}

			RayGizmo( float32 length )
			    : Gizmo( Color::WHITE(), GizmoType::RAY )
			    , _length( length )
			{
			}

			RayGizmo( const RayGizmoConfiguration* configuration )
			    : Gizmo( configuration )
			    , _length( configuration->length )
			{
			}

			RayGizmo( const RayGizmo& other ) = default;
			RayGizmo( RayGizmo&& other ) noexcept = default;

			RayGizmo& operator=( const RayGizmo& other ) = default;
			RayGizmo& operator=( RayGizmo&& other ) noexcept = default;

			float32 GetLength() const { return _length; }

			RayGizmo* Clone() const override;

		protected:
			void ConfigureConcrete( const GizmoConfiguration* configuration ) override;

		private:
			float32 _length;
	};
} // namespace Engine
