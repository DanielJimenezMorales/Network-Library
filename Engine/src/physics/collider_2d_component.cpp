#include "collider_2d_component.h"

// TODO remove this
#include "physics/circle_bounds_2d.h"

#include "logger.h"

namespace Engine
{
	CollisionResponseType GetCollisionResponseTypeFromName( const std::string& name )
	{
		if ( name == "Static" )
		{
			return CollisionResponseType::Static;
		}
		else if ( name == "Dynamic" )
		{
			return CollisionResponseType::Dynamic;
		}
		else
		{
			LOG_ERROR( "[GetCollisionResponseTypeFromName] : Can't parse %s to collision response type", name.c_str() );
		}

		return CollisionResponseType::None;
	}

	Collider2DComponent::Collider2DComponent()
	    // TODO Do not hardcode this
	    : _bounds( new CircleBounds2D( 5.f ) )
	    , _isTrigger( true )
	    , _collisionResponseType( CollisionResponseType::Static )
	{
	}

	Collider2DComponent::Collider2DComponent( Bounds2D* bounds, bool isTrigger, CollisionResponseType responseType )
	    : _bounds( bounds )
	    , _isTrigger( isTrigger )
	    , _collisionResponseType( responseType )
	{
	}

	Collider2DComponent::Collider2DComponent( Collider2DComponent&& other ) noexcept
	    : _bounds( std::exchange( other._bounds, nullptr ) )
	    , _isTrigger( other._isTrigger )
	    , _collisionResponseType( other._collisionResponseType )
	{
	}

	Collider2DComponent::~Collider2DComponent()
	{
		if ( _bounds != nullptr )
		{
			delete ( _bounds );
		}
	}

	Collider2DComponent& Collider2DComponent::operator=( Collider2DComponent&& other ) noexcept
	{
		if ( this == &other )
		{
			return *this;
		}

		if ( _bounds != nullptr )
		{
			delete ( _bounds );
		}

		_bounds = std::exchange( other._bounds, nullptr );
		_isTrigger = other._isTrigger;
		_collisionResponseType = other._collisionResponseType;
		return *this;
	}

	void Collider2DComponent::GetAxes( const TransformComponent& transform, std::vector< Vec2f >& outAxes ) const
	{
		_bounds->GetAxes( transform, outAxes );
	}

	void Collider2DComponent::ProjectAxis( const TransformComponent& transform, const Vec2f& axis, float& outMin,
	                                       float& outMax ) const
	{
		_bounds->ProjectAxis( transform, axis, outMin, outMax );
	}

	Vec2f Collider2DComponent::GetClosestVertex( const TransformComponent& transform, const Vec2f& inputPoint ) const
	{
		return _bounds->GetClosestVertex( transform, inputPoint );
	}

	float32 Collider2DComponent::GetMinX( const TransformComponent& transform ) const
	{
		return _bounds->GetMinX( transform );
	}

	float32 Collider2DComponent::GetMaxX( const TransformComponent& transform ) const
	{
		return _bounds->GetMaxX( transform );
	}

	std::unique_ptr< GizmoConfiguration > Collider2DComponent::GetGizmo() const
	{
		return std::move( _bounds->GetGizmo() );
	}
} // namespace Engine
