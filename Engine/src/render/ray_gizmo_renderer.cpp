#include "ray_gizmo_renderer.h"

#include <cassert>

#include "vec2f.h"
#include "coordinates_conversion_utils.h"
#include "camera_component.h"

#include "transform/transform_component.h"
#include "transform/transform_hierarchy_helper_functions.h"

#include "render/gizmo.h"
#include "render/ray_gizmo.h"
#include "render/color.h"

namespace Engine
{
	RayGizmoRenderer::RayGizmoRenderer()
	    : GizmoRenderer()
	{
	}

	void RayGizmoRenderer::Render( const Gizmo& gizmo, const CameraComponent& camera,
	                               const TransformComponent& camera_transform, const TransformComponent& transform,
	                               SDL_Renderer* renderer ) const
	{
		assert( gizmo.GetType() == GizmoType::RAY );

		const TransformComponentProxy transformComponentProxy;

		const RayGizmo& ray_gizmo = static_cast< const RayGizmo& >( gizmo );

		const Vec2f start_position = transformComponentProxy.GetGlobalPosition( transform );
		const Vec2f direction = transformComponentProxy.GetForwardVector( transform );
		const Vec2f end_position =
		    transformComponentProxy.GetGlobalPosition( transform ) + ( ray_gizmo.GetLength() * direction );

		const Vec2f screen_start_position =
		    ConvertFromWorldPositionToScreenPosition( start_position, camera, camera_transform );
		const Vec2f screen_end_position =
		    ConvertFromWorldPositionToScreenPosition( end_position, camera, camera_transform );

		const Color& color = ray_gizmo.GetColor();
		SDL_SetRenderDrawColor( renderer, color.R(), color.G(), color.B(), color.A() );
		SDL_RenderDrawLine( renderer, static_cast< int32 >( screen_start_position.X() ),
		                    static_cast< int32 >( screen_start_position.Y() ),
		                    static_cast< int32 >( screen_end_position.X() ),
		                    static_cast< int32 >( screen_end_position.Y() ) );
	}
}
