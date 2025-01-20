#pragma once
#include "numeric_types.h"
#include "logger.h"

#include "Gizmo.h"

#include <vector>

class Gizmo;

struct GizmoHandler
{
		GizmoHandler( uint32 index, uint32 version, GizmoType type )
		    : index( index )
		    , version( version )
		    , type( type )
		{
		}

		static GizmoHandler GetInvalid() { return GizmoHandler( 0, 0, GizmoType::NONE ); }

		bool IsValid() const { return type != GizmoType::NONE; }

		uint32 index;
		uint32 version;
		GizmoType type;
};

struct GizmoSlot
{
		GizmoSlot( Gizmo* gizmo )
		    : gizmo( gizmo )
		    , version( 0 )
		{
		}

		Gizmo* gizmo;
		uint32 version;
};

class GizmoPool
{
	public:
		GizmoPool();
		GizmoPool( const Gizmo& item, uint32 initial_size );
		GizmoPool( const GizmoPool& ) = delete;

		~GizmoPool();

		GizmoPool& operator=( const GizmoPool& ) = delete;

		bool IsFull() const;

		GizmoHandler CreateGizmo( const GizmoConfiguration* configuration );
		bool RemoveGizmo( const GizmoHandler& handler );

		const Gizmo* TryGetGizmoFromHandler( const GizmoHandler& handler ) const;
		Gizmo* TryGetGizmoFromHandler( const GizmoHandler& handler );

	private:
		void Allocate( const Gizmo& item, uint32 size );
		void Reallocate();
		void Free();

		bool TryGetEmptySlot( uint32& index ) const;

		std::vector< GizmoSlot > gizmoPool;
		// True on empty, otherwise false
		std::vector< bool > areEmpty;
};
