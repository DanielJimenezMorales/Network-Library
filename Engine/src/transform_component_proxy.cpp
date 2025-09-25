#include "transform_component_proxy.h"

#include "math_utils.h"

#include "components/transform_component.h"

namespace Engine
{
	TransformComponentProxy::TransformComponentProxy( ECS::GameEntity& entity )
	    : _entity( entity )
	    , _transformComponent( nullptr )
	    , _helperFunctions()
	{
		assert( entity.IsValid() );
		assert( entity.HasComponent< TransformComponent >() );

		_transformComponent = &entity.GetComponent< TransformComponent >();
	}

	Vec2f TransformComponentProxy::GetGlobalPosition()
	{
		return _helperFunctions.GetGlobalPosition( *_transformComponent );
	}

	void TransformComponentProxy::SetGlobalPosition( const Vec2f& new_position )
	{
		_helperFunctions.SetGlobalPosition( *_transformComponent, new_position );
	}

	Vec2f TransformComponentProxy::GetLocalPosition()
	{
		return _helperFunctions.GetLocalPosition( *_transformComponent );
	}

	void TransformComponentProxy::SetLocalPosition( const Vec2f& new_local_position )
	{
		_helperFunctions.SetLocalPosition( *_transformComponent, new_local_position );
	}

	float32 TransformComponentProxy::GetGlobalRotation()
	{
		return _helperFunctions.GetGlobalRotation( *_transformComponent );
	}

	void TransformComponentProxy::SetGlobalRotationAngle( float32 new_angle )
	{
		_helperFunctions.SetGlobalRotationAngle( *_transformComponent, new_angle );
	}

	float32 TransformComponentProxy::GetLocalRotationAngle()
	{
		return _helperFunctions.GetLocalRotationAngle( *_transformComponent );
	}

	void TransformComponentProxy::SetLocalRotationAngle( float32 new_local_angle )
	{
		_helperFunctions.SetLocalRotationAngle( *_transformComponent, new_local_angle );
	}

	void TransformComponentProxy::SetRotationLookAt( Vec2f look_at_direction )
	{
		look_at_direction.Normalize();
		_helperFunctions.SetRotationLookAt( *_transformComponent, look_at_direction );
	}

	void TransformComponentProxy::LookAt( const Vec2f& position )
	{
		_helperFunctions.LookAt( *_transformComponent, position );
	}

	Vec2f TransformComponentProxy::GetForwardVector()
	{
		return _helperFunctions.GetForwardVector( *_transformComponent );
	}

	Vec2f TransformComponentProxy::GetGlobalScale()
	{
		return _helperFunctions.GetGlobalScale( *_transformComponent );
	}

	void TransformComponentProxy::SetGlobalScale( const Vec2f& new_scale )
	{
		_helperFunctions.SetGlobalScale( *_transformComponent, new_scale );
	}

	Vec2f TransformComponentProxy::GetLocalScale()
	{
		return _helperFunctions.GetLocalScale( *_transformComponent );
	}

	void TransformComponentProxy::SetLocalScale( const Vec2f& new_local_scale )
	{
		_helperFunctions.SetLocalScale( *_transformComponent, new_local_scale );
	}

	void TransformComponentProxy::RemoveParent()
	{
		_helperFunctions.RemoveParent( *_transformComponent, _entity );
	}

	void TransformComponentProxy::SetParent( ECS::GameEntity& parent_entity )
	{
		_helperFunctions.SetParent( *_transformComponent, _entity, parent_entity );
	}

	bool TransformComponentProxy::HasParent() const
	{
		return _helperFunctions.HasParent( *_transformComponent );
	}

	bool TransformComponentProxy::HasChildren() const
	{
		return _helperFunctions.HasChildren( *_transformComponent );
	}

	const std::vector< ECS::GameEntity >& TransformComponentProxy::GetChildren() const
	{
		return _helperFunctions.GetChildren( *_transformComponent );
	}

	std::vector< ECS::GameEntity > TransformComponentProxy::GetChildren()
	{
		return _helperFunctions.GetChildren( *_transformComponent );
	}

	void TransformComponentProxy::SetChildrenDirty()
	{
		_helperFunctions.SetChildrenDirty( *_transformComponent );
	}

	void TransformComponentProxy::ResolveDirty()
	{
		return _helperFunctions.ResolveDirty( *_transformComponent );
	}

	bool TransformComponentProxy::IsDirty() const
	{
		return _helperFunctions.IsDirty( *_transformComponent );
	}
} // namespace Engine
