#include "gizmo_pool.h"

#include <cassert>

GizmoPool::GizmoPool()
    : gizmoPool()
    , areEmpty()
{
}

GizmoPool::GizmoPool( const Gizmo& item, uint32 initial_size )
    : gizmoPool()
    , areEmpty()
{
	Allocate( item, initial_size );
}

GizmoPool::~GizmoPool()
{
	Free();
}

bool GizmoPool::IsFull() const
{
	bool result = true;

	for ( auto cit = areEmpty.cbegin(); cit != areEmpty.cend(); ++cit )
	{
		if ( *cit )
		{
			result = false;
			break;
		}
	}
	return result;
}

GizmoHandler GizmoPool::CreateGizmo( const GizmoConfiguration& configuration )
{
	GizmoHandler result = GizmoHandler::GetInvalid();

	assert( !IsFull() );
	if ( IsFull() )
	{
		LOG_WARNING( "Gizmo pool for type %u is full. Reallocating a bigger pool...",
		             static_cast< uint32 >( configuration.type ) );
		Reallocate();
	}

	// Get index
	uint32 index;
	const bool index_found_succesfully = TryGetEmptySlot( index );
	assert( index_found_succesfully );

	// Assign gizmo to pool slot
	areEmpty[ index ] = false;
	gizmoPool[ index ].gizmo->Configure( configuration );
	++gizmoPool[ index ].version;

	// Create handler
	result.index = index;
	result.version = gizmoPool[ index ].version;
	result.type = configuration.type;

	return result;
}

bool GizmoPool::RemoveGizmo( const GizmoHandler& handler )
{
	bool result = false;

	assert( handler.index < gizmoPool.size() );

	if ( !areEmpty[ handler.index ] )
	{
		areEmpty[ handler.index ] = true;
		result = true;
	}

	return result;
}

const Gizmo* GizmoPool::TryGetGizmoFromHandler( const GizmoHandler& handler ) const
{
	assert( handler.index < gizmoPool.size() );

	Gizmo* result = nullptr;
	if ( !areEmpty[ handler.index ] && gizmoPool[ handler.index ].version == handler.version )
	{
		result = gizmoPool[ handler.index ].gizmo;
	}

	return result;
}

Gizmo* GizmoPool::TryGetGizmoFromHandler( const GizmoHandler& handler )
{
	return const_cast< Gizmo* >( static_cast< const GizmoPool& >( *this ).TryGetGizmoFromHandler( handler ) );
}

void GizmoPool::Allocate( const Gizmo& item, uint32 size )
{
	gizmoPool.reserve( size );
	areEmpty.reserve( size );

	for ( uint32 i = 0; i < size; ++i )
	{
		gizmoPool.push_back( item.Clone() );
	}

	areEmpty.assign( size, true );
}

void GizmoPool::Reallocate()
{
	const uint32 new_size = gizmoPool.size() * 2;
}

void GizmoPool::Free()
{
	for ( auto it = gizmoPool.begin(); it != gizmoPool.end(); ++it )
	{
		delete it->gizmo;
	}

	gizmoPool.clear();
	areEmpty.clear();
}

bool GizmoPool::TryGetEmptySlot( uint32& index ) const
{
	bool result = false;

	for ( uint32 i = 0; i < areEmpty.size(); ++i )
	{
		if ( areEmpty[ i ] )
		{
			index = i;
			result = true;
			break;
		}
	}
	return result;
}
