#include "raycaster.h"

#include "components/transform_component.h"
#include "components/collider_2d_component.h"

#include "CircleBounds2D.h"

/// <summary>
/// The algorithm here has been used from the section 5.3.2 Intersecting Ray or Segment Against Sphere from the book
/// https://www.r-5.org/files/books/computers/algo-list/realtime-3d/Christer_Ericson-Real-Time_Collision_Detection-EN.pdf
/// </summary>
bool PerformRaycastAgainstSphere( const Ray& ray, const TransformComponent& circle_transform,
                                  const CircleBounds2D& circle_collider, RaycastResult& out_result )
{
	bool has_collided = false;

	// Cache Ray data
	const Vec2f ray_origin = ray.origin;
	const Vec2f ray_direction = ray.direction.GetNormalize();
	const float32 ray_squared_max_distance = ray.maxDistance * ray.maxDistance;

	// Cache circle data
	const float32 circle_radius = circle_collider.GetRadius();
	const Vec2f circle_position = circle_transform.GetPosition();

	// Apply second grade formula
	// second_grade_formula_result = -b -sqrt(b^2 - c) where b = Dot(m,d), m = x0 - C and c = Dot(m, m) - r^2
	// First, we need to calculate discriminant result (What is inside the sqrt)
	const Vec2f m = ray_origin - circle_position;
	const float32 b = m.Dot( ray_direction );
	const float32 c = m.Dot( m ) - ( circle_radius * circle_radius );
	const float32 second_grade_formula_discriminant = ( b * b ) - c;

	// If discriminant < 0 it means no collision happens.
	// If discriminant == 0 it means there is one collision point.
	// If discriminant > 0 it means there are two collision points.
	if ( second_grade_formula_discriminant >= 0.f )
	{
		// Calculate second grade formula result (Only calculate the - path result as it is the closest point)
		const float32 second_grade_formula_result = ( -1.f * b ) - sqrtf( second_grade_formula_discriminant );

		// Substitute in ray formula to get collision point. Ray formula: x = x0 + t*d
		const Vec2f collision_position = ray_origin + ( second_grade_formula_result * ray_direction );

		//Check if collision point is more far away than max distance
		const float32 squared_distance = Vec2f::GetSquareDistance( collision_position, ray_origin );
		if ( squared_distance <= ray_squared_max_distance )
		{
			// Gather data
			out_result.position = collision_position;
			out_result.squaredDistance = Vec2f::GetSquareDistance( collision_position, ray_origin );
			has_collided = true;
		}
	}

	return has_collided;
}

RaycastResult ExecuteRaycast( const Ray& ray, const std::vector< ECS::GameEntity >& entities_with_colliders )
{
	RaycastResult result;

	for ( auto cit = entities_with_colliders.cbegin(); cit != entities_with_colliders.cend(); ++cit )
	{
	}

	return result;
}
