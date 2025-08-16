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
} // namespace Engine
