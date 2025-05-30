#pragma once
#include <SDL.h>

namespace Engine
{
	struct RenderGlobalComponent
	{
			SDL_Window* window;
			SDL_Renderer* renderer;
	};
}
