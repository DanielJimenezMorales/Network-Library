#include "read_only_transform_component_proxy.h"

#include "math_utils.h"

#include "components/transform_component.h"

namespace Engine
{
	ReadOnlyTransformComponentProxy::ReadOnlyTransformComponentProxy( const ECS::GameEntity& entity )
	    : _entity( entity )
	    , _transformComponent( nullptr )
	{
		assert( entity.IsValid() );
		assert( entity.HasComponent< TransformComponent >() );

		_transformComponent = &entity.GetComponent< TransformComponent >();
	}

	Vec2f ReadOnlyTransformComponentProxy::GetGlobalPosition()
	{
		return _transformComponent->_position;
	}

	float32 ReadOnlyTransformComponentProxy::GetGlobalRotationAngle()
	{
		return _transformComponent->_rotationAngle;
	}

	Vec2f ReadOnlyTransformComponentProxy::GetForwardVector() const
	{
		return ConvertAngleToNormalizedDirection( _transformComponent->_rotationAngle );
	}

	Vec2f ReadOnlyTransformComponentProxy::GetGlobalScale()
	{
		return _transformComponent->_scale;
	}
} // namespace Engine
