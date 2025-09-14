#pragma once
#include <SDL.h>

namespace Engine
{
	class ReadOnlyTransformComponentProxy;
	struct CameraComponent;
	class Gizmo;

	class GizmoRenderer
	{
		public:
			GizmoRenderer() {}
			~GizmoRenderer() {}

			virtual void Render( const Gizmo& gizmo, const CameraComponent& camera,
			                     ReadOnlyTransformComponentProxy& camera_transform,
			                     ReadOnlyTransformComponentProxy& transform, SDL_Renderer* renderer ) const = 0;
	};
}
