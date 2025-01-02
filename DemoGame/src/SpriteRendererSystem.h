#pragma once
#include <SDL_image.h>

namespace ECS
{
	class EntityContainer;
}

class SpriteRendererSystem
{
	public:
		void Render( ECS::EntityContainer& entityContainer, SDL_Renderer* renderer ) const;
};
