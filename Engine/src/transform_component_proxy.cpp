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
		if ( _transformComponent->_isDirty )
		{
			ResolveDirty();
		}

		_transformComponent->_isDirty = true;
		return _transformComponent->_position;
	}

	void TransformComponentProxy::SetGlobalPosition( const Vec2f& new_position )
	{
		if ( _transformComponent->_isDirty )
		{
			ResolveDirty();
		}

		_transformComponent->_position = new_position;
		SetChildrenDirty();
	}

	float32 TransformComponentProxy::GetGlobalRotation()
	{
		if ( _transformComponent->_isDirty )
		{
			ResolveDirty();
		}

		return _transformComponent->_rotationAngle;
	}

	void TransformComponentProxy::SetGlobalRotationAngle( float32 new_angle )
	{
		if ( _transformComponent->_isDirty )
		{
			ResolveDirty();
		}

		_transformComponent->_rotationAngle = std::fmodf( new_angle, 360.0f );
		if ( _transformComponent->_rotationAngle < 0.f )
		{
			_transformComponent->_rotationAngle += 360.f; // Ensure the angle is always positive
		}

		SetChildrenDirty();
	}

	void TransformComponentProxy::SetRotationLookAt( Vec2f look_at_direction )
	{
		look_at_direction.Normalize();

		SetGlobalRotationAngle( ConvertNormalizedDirectionToAngle( look_at_direction ) );
	}

	void TransformComponentProxy::LookAt( const Vec2f& position )
	{
		const Vec2f direction = position - GetGlobalPosition();
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

		SetGlobalRotationAngle( GetGlobalRotation() + angleInDegrees );
	}

	Vec2f TransformComponentProxy::GetForwardVector()
	{
		return ConvertAngleToNormalizedDirection( GetGlobalRotation() );
	}

	Vec2f TransformComponentProxy::GetGlobalScale()
	{
		if ( _transformComponent->_isDirty )
		{
			ResolveDirty();
		}

		return _transformComponent->_scale;
	}

	void TransformComponentProxy::SetGlobalScale( const Vec2f& new_scale )
	{
		if ( _transformComponent->_isDirty )
		{
			ResolveDirty();
		}

		_transformComponent->_scale = new_scale;
		SetChildrenDirty();
	}

	void TransformComponentProxy::RemoveParent()
	{
		if ( _transformComponent->_parent.IsValid() )
		{
			// Remove child from parent
			TransformComponent& parentTransform = _transformComponent->_parent.GetComponent< TransformComponent >();
			auto cit = parentTransform._children.cbegin();
			for ( ; cit != parentTransform._children.cend(); ++cit )
			{
				if ( *cit == _entity )
				{
					break;
				}
			}

			assert( cit != parentTransform._children.cend() );
			parentTransform._children.erase( cit );

			// Remove parent from current
			_transformComponent->_parent = ECS::GameEntity();

			_transformComponent->_isDirty = false;
		}
	}

	void TransformComponentProxy::SetParent( ECS::GameEntity& parent_entity )
	{
		// If it is dirty, resolve it before removing the parent in order to have the updated values
		if ( _transformComponent->_isDirty )
		{
			ResolveDirty();
		}

		// Remove the parent
		if ( _transformComponent->_parent.IsValid() )
		{
			RemoveParent();
		}

		// Set current as child parent
		TransformComponent& parentTransform = parent_entity.GetComponent< TransformComponent >();
		parentTransform._children.push_back( _entity );

		// Set parent as current parent
		_transformComponent->_parent = parent_entity;

		// If parent is dirty, then child must be dirty too
		_transformComponent->_isDirty = parentTransform._isDirty;
	}

	bool TransformComponentProxy::HasChildren() const
	{
		return !_transformComponent->_children.empty();
	}

	const std::vector< ECS::GameEntity >& TransformComponentProxy::GetChildren() const
	{
		return _transformComponent->_children;
	}

	std::vector< ECS::GameEntity > TransformComponentProxy::GetChildren()
	{
		return _transformComponent->_children;
	}

	void TransformComponentProxy::SetChildrenDirty()
	{
		auto childrenIt = _transformComponent->_children.begin();
		for ( ; childrenIt != _transformComponent->_children.end(); ++childrenIt )
		{
			TransformComponent& childTransform = childrenIt->GetComponent< TransformComponent >();
			childTransform._isDirty = true;
			if ( childTransform._children.size() > 0 )
			{
				TransformComponentProxy childTransformProxy( *childrenIt );
				childTransformProxy.SetChildrenDirty();
			}
		}
	}

	void TransformComponentProxy::ResolveDirty()
	{
		// TODO
	}
} // namespace Engine
