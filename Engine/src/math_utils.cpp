#include "math_utils.h"

#include <cmath>

namespace Engine
{
	float32 GetNoSquareRootDistance( const Vec2f& v1, const Vec2f& v2 )
	{
		const float32 dx = v2.X() - v1.X();
		const float32 dy = v2.Y() - v1.Y();
		return ( dx * dx ) + ( dy * dy );
	}

	float32 GetDistance( const Vec2f& v1, const Vec2f& v2 )
	{
		const float32 dx = v2.X() - v1.X();
		const float32 dy = v2.Y() - v1.Y();
		return sqrtf( ( dx * dx ) + ( dy * dy ) );
	}

	Vec2f ConvertAngleToNormalizedDirection( float32 angle )
	{
		const float32 angle_in_radians = angle * PI / 180.f;
		return Vec2f( std::cosf( angle_in_radians ), std::sinf( angle_in_radians ) );
	}

	float32 ConvertNormalizedDirectionToAngle( const Vec2f& normalized_direction )
	{
		const float32 angle_in_radians = std::atan2f( normalized_direction.Y(), normalized_direction.X() );
		return angle_in_radians * 180.f / PI;
	}

	float32 CalculateRotationAngleToTarget( const Vec2f& current_position, const Vec2f& current_forward_vector,
	                                        const Vec2f& target_position )
	{
		const Vec2f direction = target_position - current_position;
		if ( direction == Vec2f( 0, 0 ) )
		{
			return 0.f;
		}

		// FORMULA of angle between two vectors: Cos(angle) = dotProduct(V1, V1) / (Mag(V1) * Mag(V2)) --> angle =
		// arcos(dotProduct(V1, V1) / (Mag(V1) * Mag(V2)))

		const float32 dotProduct =
		    ( direction.X() * current_forward_vector.X() ) + ( direction.Y() * current_forward_vector.Y() );
		float32 angleCosine = dotProduct / ( direction.Magnitude() * current_forward_vector.Magnitude() );

		// Check it because due to floating point precision error, it could happen.
		if ( angleCosine > 1.f )
		{
			angleCosine = 1.f;
		}
		else if ( angleCosine < -1.f )
		{
			angleCosine = -1.f;
		}

		float32 angleInRadians = std::acosf( angleCosine );

		// Calculate rotation direction. Since our system has an anti-clockwise rotation direction, we need to invert
		// the angle in case the cross product is positive.
		const float32 crossProduct =
		    ( direction.X() * current_forward_vector.Y() ) - ( current_forward_vector.X() * direction.Y() );
		if ( crossProduct > 0.f )
		{
			angleInRadians = -angleInRadians;
		}

		return angleInRadians * ( 180.f / PI );
	}
} // namespace Engine
