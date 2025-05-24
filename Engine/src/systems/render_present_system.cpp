#include "render_present_system.h"

#include "global_components/render_global_component.h"

#include "ecs/world.h"

namespace Engine
{
	RenderPresentSystem::RenderPresentSystem()
	    : ECS::ISimpleSystem()
	{
	}

	void Engine::RenderPresentSystem::Execute( ECS::World& world, float32 elapsed_time )
	{
		RenderGlobalComponent& renderGlobalComponent = world.GetGlobalComponent< RenderGlobalComponent >();
		SDL_RenderPresent( renderGlobalComponent.renderer );
	}
}
