#pragma once
#include "resource_handlers/gizmo_pool.h"

namespace Engine
{
	struct GizmoRendererComponent
	{
			GizmoRendererComponent( const GizmoHandler& handler )
			    : gizmoHandler( handler )
			{
			}

			GizmoHandler gizmoHandler;
	};
}
