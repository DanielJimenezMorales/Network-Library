#include "ray_gizmo.h"

#include <cassert>

RayGizmo* RayGizmo::Clone() const
{
	return new RayGizmo( *this );
}

void RayGizmo::ConfigureConcrete( const GizmoConfiguration* configuration )
{
	assert( configuration->type == GizmoType::RAY );

	const RayGizmoConfiguration& config = static_cast< const RayGizmoConfiguration& >( *configuration );
	_length = config.length;
}
