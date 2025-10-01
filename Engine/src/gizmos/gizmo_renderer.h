#pragma once
#include <SDL.h>

namespace Engine
{
	struct TransformComponent;
	struct CameraComponent;
	class Gizmo;

	class GizmoRenderer
	{
		public:
			GizmoRenderer() {}
			~GizmoRenderer() {}

			virtual void Render( const Gizmo& gizmo, const CameraComponent& camera,
			                     const TransformComponent& camera_transform, const TransformComponent& transform,
			                     SDL_Renderer* renderer ) const = 0;
	};
}
