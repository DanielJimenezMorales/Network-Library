#include "render_clear_system.h"

#include "render/render_global_component.h"

#include "ecs/world.h"

namespace Engine
{
	RenderClearSystem::RenderClearSystem()
	    : ECS::ISimpleSystem()
	{
	}

	void Engine::RenderClearSystem::Execute( ECS::World& world, float32 elapsed_time )
	{
		RenderGlobalComponent& renderGlobalComponent = world.GetGlobalComponent< RenderGlobalComponent >();
		SDL_RenderClear( renderGlobalComponent.renderer );
	}
}
