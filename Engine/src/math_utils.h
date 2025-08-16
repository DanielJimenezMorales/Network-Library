#pragma once
#include "vec2f.h"

namespace Engine
{
	constexpr float32 PI = 3.141592653589793f;

	float32 GetNoSquareRootDistance( const Vec2f& v1, const Vec2f& v2 );
	float32 GetDistance( const Vec2f& v1, const Vec2f& v2 );

	/// <summary>
	/// <para>LEFT = 180</para>
	/// <para>RIGHT = 0</para>
	/// <para>UP = 90</para>
	/// <para>DOWN = 270</para>
	/// </summary>
	Vec2f ConvertAngleToNormalizedDirection( float32 angle );

	/// <summary>
	/// <para>LEFT = 180</para>
	/// <para>RIGHT = 0</para>
	/// <para>UP = 90</para>
	/// <para>DOWN = 270</para>
	/// </summary>
	float32 ConvertNormalizedDirectionToAngle( const Vec2f& normalized_direction );

	constexpr int32 round_to_int32( float32 value )
	{
		// Note: static_cast-ing a float to an int simply get rid of the decimal part. i.e. 1.9f = 1, -3.8f = -3.
		if ( value >= 0.f )
		{
			return static_cast< int32 >( value + 0.5f );
		}
		else
		{
			return static_cast< int32 >( value - 0.5f );
		}
	};

	constexpr uint32 round_down_to_uint32( float32 value )
	{
		// Note: static_cast-ing a float to an int simply get rid of the decimal part. i.e. 1.9f = 1.
		return static_cast< uint32 >( value );
	};
} // namespace Engine
