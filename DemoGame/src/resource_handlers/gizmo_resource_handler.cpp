#include "gizmo_resource_handler.h"

#include "Circle2DGizmo.h"

#include <cassert>

GizmoResourceHandler::GizmoResourceHandler()
    : _pools()
{
	Allocate( 1024 );
}

GizmoResourceHandler::GizmoResourceHandler( GizmoResourceHandler&& other ) noexcept
    : _pools( std::move( other._pools ) )
{
}

GizmoResourceHandler::~GizmoResourceHandler()
{
	Free();
}

GizmoResourceHandler& GizmoResourceHandler::operator=( GizmoResourceHandler&& other ) noexcept
{
	if ( this == &other )
	{
		return *this;
	}

	Free();
	_pools = std::move( other._pools );

	return *this;
}

GizmoHandler GizmoResourceHandler::CreateGizmo( const GizmoConfiguration& configuration )
{
	GizmoPool* pool = ForceGetGizmoPool( configuration.type );
	return ( pool != nullptr ) ? pool->CreateGizmo( configuration ) : GizmoHandler::GetInvalid();
}

bool GizmoResourceHandler::RemoveGizmo( const GizmoHandler& handler )
{
	GizmoPool* pool = ForceGetGizmoPool( handler.type );
	return ( pool != nullptr ) ? pool->RemoveGizmo( handler ) : false;
}

const Gizmo* GizmoResourceHandler::TryGetGizmoFromHandler( const GizmoHandler& handler ) const
{
	assert( handler.IsValid() );

	const GizmoPool* pool = ForceGetGizmoPool( handler.type );
	return ( pool != nullptr ) ? pool->TryGetGizmoFromHandler( handler ) : nullptr;
}

Gizmo* GizmoResourceHandler::TryGetGizmoFromHandler( const GizmoHandler& handler )
{
	return const_cast< Gizmo* >(
	    static_cast< const GizmoResourceHandler& >( *this ).TryGetGizmoFromHandler( handler ) );
}

void GizmoResourceHandler::Allocate( uint32 number )
{
	// TODO This allocation process is hardcoded since anytime I add a new gizmo type I need to modify here
	_pools.reserve( 1 );

	CircleGizmo circle_gizmo;
	_pools.try_emplace( GizmoType::CIRCLE2D, circle_gizmo, number );
}

void GizmoResourceHandler::Free()
{
	_pools.clear();
}

const GizmoPool* GizmoResourceHandler::ForceGetGizmoPool( GizmoType type ) const
{
	const GizmoPool* pool = nullptr;

	auto pool_found = _pools.find( type );
	if ( pool_found != _pools.end() )
	{
		pool = &pool_found->second;
	}
	else
	{
		LOG_ERROR( "Gizmo pool of type %u not found", static_cast< uint32 >( type ) );
	}

	return pool;
}

GizmoPool* GizmoResourceHandler::ForceGetGizmoPool( GizmoType type )
{
	return const_cast< GizmoPool* >( static_cast< const GizmoResourceHandler& >( *this ).ForceGetGizmoPool( type ) );
}
