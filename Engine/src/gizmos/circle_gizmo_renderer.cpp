#include "circle_gizmo_renderer.h"

#include <cassert>

#include "numeric_types.h"
#include "vec2f.h"
#include "coordinates_conversion_utils.h"

#include "components/transform_component.h"
#include "components/camera_component.h"

#include "gizmos/gizmo.h"
#include "gizmos/circle_gizmo.h"

namespace Engine
{
	CircleGizmoRenderer::CircleGizmoRenderer()
	    : GizmoRenderer()
	{
	}

	void CircleGizmoRenderer::Render( const Gizmo& gizmo, const CameraComponent& camera,
	                                  const TransformComponent& camera_transform, const TransformComponent& transform,
	                                  SDL_Renderer* renderer ) const
	{
		assert( gizmo.GetType() == GizmoType::CIRCLE2D );

		const CircleGizmo& circle_gizmo = static_cast< const CircleGizmo& >( gizmo );
		const Vec2f position = transform.GetPosition();

		// Start at the top of the circle
		float32 x = circle_gizmo.GetRadius();
		float32 y = 0;

		// Decision parameter (initial value)
		float32 decisionOver2 = 1 - x;

		SDL_SetRenderDrawColor( renderer, circle_gizmo.GetR(), circle_gizmo.GetG(), circle_gizmo.GetB(),
		                        circle_gizmo.GetA() );

		// We loop until x < y, covering one octant
		while ( y <= x )
		{
			// Draw the points in all 8 octants
			Vec2f renderPosition =
			    ConvertFromWorldPositionToScreenPosition( position + Vec2f( x, y ), camera, camera_transform );
			SDL_RenderDrawPoint( renderer, renderPosition.X(), renderPosition.Y() );
			renderPosition =
			    ConvertFromWorldPositionToScreenPosition( position + Vec2f( -x, y ), camera, camera_transform );
			SDL_RenderDrawPoint( renderer, renderPosition.X(), renderPosition.Y() );
			renderPosition =
			    ConvertFromWorldPositionToScreenPosition( position + Vec2f( x, -y ), camera, camera_transform );
			SDL_RenderDrawPoint( renderer, renderPosition.X(), renderPosition.Y() );
			renderPosition =
			    ConvertFromWorldPositionToScreenPosition( position + Vec2f( -x, -y ), camera, camera_transform );
			SDL_RenderDrawPoint( renderer, renderPosition.X(), renderPosition.Y() );
			renderPosition =
			    ConvertFromWorldPositionToScreenPosition( position + Vec2f( y, x ), camera, camera_transform );
			SDL_RenderDrawPoint( renderer, renderPosition.X(), renderPosition.Y() );
			renderPosition =
			    ConvertFromWorldPositionToScreenPosition( position + Vec2f( -y, x ), camera, camera_transform );
			SDL_RenderDrawPoint( renderer, renderPosition.X(), renderPosition.Y() );
			renderPosition =
			    ConvertFromWorldPositionToScreenPosition( position + Vec2f( y, -x ), camera, camera_transform );
			SDL_RenderDrawPoint( renderer, renderPosition.X(), renderPosition.Y() );
			renderPosition =
			    ConvertFromWorldPositionToScreenPosition( position + Vec2f( -y, -x ), camera, camera_transform );
			SDL_RenderDrawPoint( renderer, renderPosition.X(), renderPosition.Y() );

			// Move to the next pixel vertically
			y++;

			// Update the decision parameter based on the previous pixel's decision
			if ( decisionOver2 <= 0 )
			{
				// If the decision parameter is less than or equal to zero,
				// we move horizontally right (x stays the same, y increases)
				decisionOver2 += 2 * y + 1;
			}
			else
			{
				// Otherwise, we move diagonally down-right (x decreases, y increases)
				x--;
				decisionOver2 += 2 * ( y - x ) + 1;
			}
		}
	}
} // namespace Engine
