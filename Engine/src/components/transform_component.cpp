#include "transform_component.h"

#include "logger.h"
#include "math_utils.h"

#include <cmath>

namespace Engine
{
	TransformComponent::TransformComponent()
	    : _position( 0.f, 0.f )
	    , _rotationAngle( 0.f )
	    , _scale( 1.f, 1.f )
	{
	}

	TransformComponent::TransformComponent( float32 x, float32 y )
	    : _position( x, y )
	    , _rotationAngle( 0.f )
	    , _scale( 1.f, 1.f )
	{
	}

	TransformComponent::TransformComponent( const Vec2f& position, float32 rotation )
	    : _position( position )
	    , _rotationAngle( rotation )
	    , _scale( 1.f, 1.f )
	{
	}

	void TransformComponent::LookAt( const Vec2f& position )
	{
		Vec2f direction = position - _position;
		if ( direction == Vec2f( 0, 0 ) )
		{
			return;
		}

		Vec2f forward = GetForwardVector();

		// FORMULA of angle between two vectors: Cos(angle) = dotProduct(V1, V1) / (Mag(V1) * Mag(V2)) --> angle =
		// arcos(dotProduct(V1, V1) / (Mag(V1) * Mag(V2)))

		const float32 dotProduct = ( direction.X() * forward.X() ) + ( direction.Y() * forward.Y() );
		float32 angleCosine = dotProduct / ( direction.Magnitude() * forward.Magnitude() );

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
		const float32 crossProduct = ( direction.X() * forward.Y() ) - ( forward.X() * direction.Y() );
		if ( crossProduct > 0.f )
		{
			angleInRadians = -angleInRadians;
		}

		const float32 angleInDegrees = angleInRadians * ( 180.f / PI );

		SetRotationAngle( _rotationAngle + angleInDegrees );
	}

	Vec2f TransformComponent::GetForwardVector() const
	{
		return ConvertAngleToNormalizedDirection( _rotationAngle );
	}

	void TransformComponent::SetRotationAngle( float32 newRotationAngle )
	{
		_rotationAngle = std::fmodf( newRotationAngle, 360.0f );
		if ( _rotationAngle < 0.f )
		{
			_rotationAngle += 360.f; // Ensure the angle is always positive
		}
	}

	void TransformComponent::SetRotationLookAt( Vec2f look_at_direction )
	{
		look_at_direction.Normalize();

		SetRotationAngle( ConvertNormalizedDirectionToAngle( look_at_direction ) );
	}
} // namespace Engine