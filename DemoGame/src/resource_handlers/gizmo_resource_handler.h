#pragma once
#include "numeric_types.h"

#include "Gizmo.h"

#include "resource_handlers/gizmo_pool.h"

#include <unordered_map>

class GizmoResourceHandler
{
	public:
		GizmoResourceHandler();
		GizmoResourceHandler( const GizmoResourceHandler& ) = delete;
		GizmoResourceHandler( GizmoResourceHandler&& other ) noexcept;

		~GizmoResourceHandler();

		GizmoResourceHandler& operator=( const GizmoResourceHandler& ) = delete;
		GizmoResourceHandler& operator=( GizmoResourceHandler&& other ) noexcept;

		GizmoHandler CreateGizmo( const GizmoConfiguration& configuration );
		bool RemoveGizmo( const GizmoHandler& handler );

		const Gizmo* TryGetGizmoFromHandler( const GizmoHandler& handler ) const;
		Gizmo* TryGetGizmoFromHandler( const GizmoHandler& handler );

	private:
		void Allocate( uint32 number );
		void Free();

		const GizmoPool* ForceGetGizmoPool( GizmoType type ) const;
		GizmoPool* ForceGetGizmoPool( GizmoType type );

		std::unordered_map< GizmoType, GizmoPool > _pools;
};
