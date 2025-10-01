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

	/// <summary>
	/// Calculates the signed rotation angle (in degrees) between the entity’s current forward vector and the direction
	/// toward the target.
	/// </summary>
	/// <param name="current_position">The position where the object is at</param>
	/// <param name="current_forward_vector">The forward vector of the object</param>
	/// <param name="target_position">The position where the object wants to look at</param>
	/// <returns>The neccessary angle to make the object look at target_position.</returns>
	float32 CalculateRotationAngleToTarget( const Vec2f& current_position, const Vec2f& current_forward_vector,
	                                        const Vec2f& target_position );

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
