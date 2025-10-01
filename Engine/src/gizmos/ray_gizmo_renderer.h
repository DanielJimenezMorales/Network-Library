#pragma once
#include "gizmos/gizmo_renderer.h"

namespace Engine
{
	class RayGizmoRenderer : public GizmoRenderer
	{
		public:
			RayGizmoRenderer();

			void Render( const Gizmo& gizmo, const CameraComponent& camera, const TransformComponent& camera_transform,
			             const TransformComponent& transform, SDL_Renderer* renderer ) const override;
	};
}
