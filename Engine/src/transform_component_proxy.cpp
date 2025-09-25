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

	Vec2f TransformComponentProxy::GetLocalPosition()
	{
		Vec2f result;
		if ( HasParent() )
		{
			result = _transformComponent->_localPosition;
		}
		else
		{
			result = _transformComponent->_position;
		}

		return result;
	}

	void TransformComponentProxy::SetLocalPosition( const Vec2f& new_local_position )
	{
		if ( HasParent() )
		{
			_transformComponent->_localPosition = new_local_position;
			_transformComponent->_isDirty = true;
		}
		else
		{
			_transformComponent->_position = new_local_position;
		}

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

	float32 TransformComponentProxy::GetLocalRotationAngle()
	{
		float32 result;
		if ( HasParent() )
		{
			result = _transformComponent->_localRotationAngle;
		}
		else
		{
			result = _transformComponent->_rotationAngle;
		}

		return result;
	}

	void TransformComponentProxy::SetLocalRotationAngle( float32 new_local_angle )
	{
		if ( HasParent() )
		{
			_transformComponent->_localRotationAngle = std::fmodf( new_local_angle, 360.0f );
			if ( _transformComponent->_localRotationAngle < 0.f )
			{
				_transformComponent->_localRotationAngle += 360.f; // Ensure the angle is always positive
			}

			_transformComponent->_isDirty = true;
		}
		else
		{
			_transformComponent->_rotationAngle = std::fmodf( new_local_angle, 360.0f );
			if ( _transformComponent->_rotationAngle < 0.f )
			{
				_transformComponent->_rotationAngle += 360.f; // Ensure the angle is always positive
			}
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

	Vec2f TransformComponentProxy::GetLocalScale()
	{
		Vec2f result;
		if ( HasParent() )
		{
			result = _transformComponent->_localScale;
		}
		else
		{
			result = _transformComponent->_scale;
		}

		return result;
	}

	void TransformComponentProxy::SetLocalScale( const Vec2f& new_local_scale )
	{
		if ( HasParent() )
		{
			_transformComponent->_localScale = new_local_scale;
			_transformComponent->_isDirty = true;
		}
		else
		{
			_transformComponent->_localScale = new_local_scale;
		}

		SetChildrenDirty();
	}

	void TransformComponentProxy::RemoveParent()
	{
		if ( _transformComponent->_parent.IsValid() )
		{
			ResolveDirty();

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

			// Reset local transform to default values
			_transformComponent->_localPosition = Vec2f( 0.f, 0.f );
			_transformComponent->_localRotationAngle = 0.f;
			_transformComponent->_localScale = Vec2f( 1.f, 1.f );

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

		// If parent is dirty, resolve it
		TransformComponentProxy parentTransformProxy( parent_entity );
		parentTransformProxy.ResolveDirty();

		// Calculate local transform based on current global transform and parent's global transform
		_transformComponent->_localPosition = GetGlobalPosition() - parentTransformProxy.GetGlobalPosition();
		_transformComponent->_localRotationAngle = GetGlobalRotation() - parentTransformProxy.GetGlobalRotation();
		Vec2f thisScale = GetGlobalScale();
		Vec2f parentScale = parentTransformProxy.GetGlobalScale();
		_transformComponent->_localScale = GetGlobalScale() - parentTransformProxy.GetGlobalScale();
	}

	bool TransformComponentProxy::HasParent() const
	{
		return _transformComponent->_parent.IsValid();
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
		if ( !_transformComponent->_isDirty || !HasParent() )
		{
			return;
		}

		// Resolve parent's dirty state first
		TransformComponentProxy parentTransformProxy( _transformComponent->_parent );
		if ( parentTransformProxy.IsDirty() )
		{
			parentTransformProxy.ResolveDirty();
		}

		// Recalculate global transform based on local transform and parent's global transform
		_transformComponent->_position = parentTransformProxy.GetGlobalPosition() + GetLocalPosition();
		_transformComponent->_rotationAngle = parentTransformProxy.GetGlobalRotation() + GetLocalRotationAngle();
		_transformComponent->_scale = parentTransformProxy.GetGlobalScale() * GetLocalScale();
		_transformComponent->_isDirty = false;
	}

	bool TransformComponentProxy::IsDirty() const
	{
		return _transformComponent->_isDirty;
	}
} // namespace Engine
