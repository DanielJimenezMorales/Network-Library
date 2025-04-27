#include "ray_gizmo.h"

#include "Vec2f.h"

#include "components/transform_component.h"
#include "components/camera_component.h"

#include "coordinates_conversion_utils.h"

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

void RayGizmo::RenderConcrete( const CameraComponent& camera, const TransformComponent& camera_transform,
                               const TransformComponent& transform, SDL_Renderer* renderer ) const
{
	const Vec2f start_position = transform.GetPosition();
	const Vec2f direction = transform.ConvertRotationAngleToNormalizedDirection();
	const Vec2f end_position = transform.GetPosition() + ( _length * direction );

	const Vec2f screen_start_position =
	    ConvertFromWorldPositionToScreenPosition( start_position, camera, camera_transform );
	const Vec2f screen_end_position =
	    ConvertFromWorldPositionToScreenPosition( end_position, camera, camera_transform );

	SDL_RenderDrawLine( renderer, screen_start_position.X(), screen_start_position.Y(), screen_end_position.X(),
	                    screen_end_position.Y() );
}
