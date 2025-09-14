#pragma once
#include "gizmos/gizmo_renderer.h"

namespace Engine
{
	class CircleGizmoRenderer : public GizmoRenderer
	{
		public:
			CircleGizmoRenderer();

			void Render( const Gizmo& gizmo, const CameraComponent& camera,
			             ReadOnlyTransformComponentProxy& camera_transform, ReadOnlyTransformComponentProxy& transform,
			             SDL_Renderer* renderer ) const override;
	};
}
