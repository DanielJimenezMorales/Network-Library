#include "circle_gizmo.h"

#include <cassert>

CircleGizmo* CircleGizmo::Clone() const
{
	return new CircleGizmo( *this );
}

void CircleGizmo::ConfigureConcrete( const GizmoConfiguration* configuration )
{
	assert( configuration->type == GizmoType::CIRCLE2D );

	const CircleGizmoConfiguration& config = static_cast< const CircleGizmoConfiguration& >( *configuration );
	_radius = config.radius;
}
