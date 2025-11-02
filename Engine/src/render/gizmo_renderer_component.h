#pragma once
#include "render/gizmo_pool.h"

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
