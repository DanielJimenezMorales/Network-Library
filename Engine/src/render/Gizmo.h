#pragma once
#include "numeric_types.h"

#include "render/color.h"

namespace Engine
{
	enum class GizmoType : uint32
	{
		NONE = 0,
		CIRCLE2D = 1,
		RAY = 2
	};

	struct GizmoConfiguration
	{
			virtual ~GizmoConfiguration() {}

			Color color;
			GizmoType type;
	};

	class Gizmo
	{
		public:
			virtual ~Gizmo() {}

			Gizmo& operator=( const Gizmo& other ) = default;
			Gizmo& operator=( Gizmo&& other ) noexcept = default;

			GizmoType GetType() const { return _type; }
			const Color& GetColor() const { return _color; }

			void Configure( const GizmoConfiguration* configuration );

			virtual Gizmo* Clone() const = 0;

		protected:
			Gizmo( const GizmoConfiguration* configuration )
			    : _color( configuration->color )
			    , _type( configuration->type )
			{
			}

			Gizmo( const Color& color, GizmoType type );
			Gizmo( const Gizmo& other ) = default;
			Gizmo( Gizmo&& other ) noexcept = default;

			virtual void ConfigureConcrete( const GizmoConfiguration* configuration ) = 0;

		private:
			Color _color;
			GizmoType _type;
	};
} // namespace Engine
