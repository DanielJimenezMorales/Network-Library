#include "read_only_transform_component_proxy.h"

#include "math_utils.h"

#include "components/transform_component.h"

namespace Engine
{
	ReadOnlyTransformComponentProxy::ReadOnlyTransformComponentProxy( const ECS::GameEntity& entity )
	    : _entity( entity )
	    , _transformComponent( nullptr )
	    , _helperFunctions()
	{
		assert( entity.IsValid() );
		assert( entity.HasComponent< TransformComponent >() );

		_transformComponent = &entity.GetComponent< TransformComponent >();
	}

	Vec2f ReadOnlyTransformComponentProxy::GetGlobalPosition()
	{
		return _helperFunctions.GetGlobalPosition( *_transformComponent );
	}

	float32 ReadOnlyTransformComponentProxy::GetGlobalRotationAngle()
	{
		return _helperFunctions.GetGlobalRotation( *_transformComponent );
	}

	Vec2f ReadOnlyTransformComponentProxy::GetForwardVector() const
	{
		return _helperFunctions.GetForwardVector( *_transformComponent );
	}

	Vec2f ReadOnlyTransformComponentProxy::GetGlobalScale()
	{
		return _helperFunctions.GetGlobalScale( *_transformComponent );
	}
} // namespace Engine
