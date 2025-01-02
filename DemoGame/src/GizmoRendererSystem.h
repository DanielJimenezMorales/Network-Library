#pragma once
#include "SDL.h"

namespace ECS
{
	class EntityContainer;
}

class GizmoRendererSystem
{
	public:
		void Render( ECS::EntityContainer& entityContainer, SDL_Renderer* renderer ) const;
};
