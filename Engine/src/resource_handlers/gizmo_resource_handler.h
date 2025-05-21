#pragma once
#include "numeric_types.h"

#include "gizmos/Gizmo.h"

#include "resource_handlers/gizmo_pool.h"

#include <unordered_map>

namespace Engine
{
	/// <summary>
	/// This class acts as a centralized storage for Gizmo resources. This class follows the Handle pattern.
	/// https://floooh.github.io/2018/06/17/handles-vs-pointers.html
	/// </summary>
	class GizmoResourceHandler
	{
		public:
			GizmoResourceHandler();
			GizmoResourceHandler( const GizmoResourceHandler& ) = delete;
			GizmoResourceHandler( GizmoResourceHandler&& other ) noexcept;

			~GizmoResourceHandler();

			GizmoResourceHandler& operator=( const GizmoResourceHandler& ) = delete;
			GizmoResourceHandler& operator=( GizmoResourceHandler&& other ) noexcept;

			/// <summary>
			/// Creates a Gizmo resource based on a configuration. Returns a handler to the resource.
			/// </summary>
			GizmoHandler CreateGizmo( const GizmoConfiguration* configuration );
			/// <summary>
			/// Removes a gizmo resource based on its handler.
			/// </summary>
			bool RemoveGizmo( const GizmoHandler& handler );

			/// <summary>
			/// <para>Returns a pointer to the raw gizmo resource based on its pointer. If the gizmo resource is not
			/// found, nullptr will be returned.</para> <para>IMPORTANT: Never store the raw resource pointer returned.
			/// Also, never pass the resource pointer to another function. If neccessary, make another call to this
			/// function.</para>
			/// </summary>
			const Gizmo* TryGetGizmoFromHandler( const GizmoHandler& handler ) const;
			Gizmo* TryGetGizmoFromHandler( const GizmoHandler& handler );

		private:
			void Allocate( uint32 number );
			void Free();

			const GizmoPool* ForceGetGizmoPool( GizmoType type ) const;
			GizmoPool* ForceGetGizmoPool( GizmoType type );

			std::unordered_map< GizmoType, GizmoPool > _pools;
	};
} // namespace Engine
