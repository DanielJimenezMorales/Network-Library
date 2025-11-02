#pragma once
#include "SDL.h"
#include "render/color.h"

namespace Engine
{
	struct RenderGlobalComponent
	{
			SDL_Window* window;
			SDL_Renderer* renderer;
			Color clearColor;
	};
}
