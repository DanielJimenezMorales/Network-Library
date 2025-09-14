#include "transform_component_proxy.h"

#include "math_utils.h"

#include "components/transform_component.h"

namespace Engine
{
	TransformComponentProxy::TransformComponentProxy( ECS::GameEntity& entity )
	    : _entity( entity )
	    , _transformComponent( nullptr )
	{
		assert( entity.IsValid() );
		assert( entity.HasComponent< TransformComponent >() );

		_transformComponent = &entity.GetComponent< TransformComponent >();
	}

	Vec2f TransformComponentProxy::GetGlobalPosition()
	{
		return _transformComponent->_position;
	}

	void TransformComponentProxy::SetGlobalPosition( const Vec2f& new_position )
	{
		_transformComponent->_position = new_position;
	}

	float32 TransformComponentProxy::GetGlobalRotation()
	{
		return _transformComponent->_rotationAngle;
	}

	void TransformComponentProxy::SetGlobalRotationAngle( float32 new_angle )
	{
		_transformComponent->_rotationAngle = std::fmodf( new_angle, 360.0f );
		if ( _transformComponent->_rotationAngle < 0.f )
		{
			_transformComponent->_rotationAngle += 360.f; // Ensure the angle is always positive
		}
	}

	void TransformComponentProxy::SetRotationLookAt( Vec2f look_at_direction )
	{
		look_at_direction.Normalize();

		SetGlobalRotationAngle( ConvertNormalizedDirectionToAngle( look_at_direction ) );
	}

	void TransformComponentProxy::LookAt( const Vec2f& position )
	{
		const Vec2f direction = position - _transformComponent->_position;
		if ( direction == Vec2f( 0, 0 ) )
		{
			return;
		}

		const Vec2f forward = GetForwardVector();

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

		SetGlobalRotationAngle( _transformComponent->_rotationAngle + angleInDegrees );
	}

	Vec2f TransformComponentProxy::GetForwardVector() const
	{
		return ConvertAngleToNormalizedDirection( _transformComponent->_rotationAngle );
	}

	Vec2f TransformComponentProxy::GetGlobalScale()
	{
		return _transformComponent->_scale;
	}

	void TransformComponentProxy::SetGlobalScale( const Vec2f& new_scale )
	{
		_transformComponent->_scale = new_scale;
	}
} // namespace Engine
