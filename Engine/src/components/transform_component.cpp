#include "transform_component.h"

#include <cmath>
#include "Logger.h"
#include "MathUtils.h"

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

	// Calculate rotation direction
	const float32 crossProduct = ( direction.X() * forward.Y() ) - ( forward.X() * direction.Y() );
	if ( crossProduct < 0.f )
	{
		angleInRadians = -angleInRadians;
	}

	const float32 angleInDegrees = angleInRadians * ( 180.f / PI );

	SetRotationAngle( _rotationAngle + angleInDegrees );
}

Vec2f TransformComponent::GetForwardVector() const
{
	float32 angleInRadians = _rotationAngle * ( PI / 180.f );

	// Since the rotation direction is anti-clockwise, we need to do the sin and cos of negative angle instead of just
	// the positive angle.
	Vec2f forwardVector( std::sin( -angleInRadians ), -std::cos( -angleInRadians ) );

	return forwardVector;
}

Vec2f TransformComponent::ConvertRotationAngleToNormalizedDirection() const
{
	return ConvertAngleToNormalizedDirection( _rotationAngle );
}

void TransformComponent::SetRotationAngle( float32 newRotationAngle )
{
	_rotationAngle = std::fmodf( newRotationAngle, 360.0f );
}

void TransformComponent::SetRotationLookAt( Vec2f look_at_direction )
{
	look_at_direction.Normalize();

	const float32 angle_in_radians = std::atan2f( look_at_direction.X(), look_at_direction.Y() );
	const float angle_in_degrees = angle_in_radians * 180.f / PI;

	//+180 degrees in order to align with Vec2f(0, 1) being assigned to 0 degrees.
	_rotationAngle = angle_in_degrees + 180.f;
}
