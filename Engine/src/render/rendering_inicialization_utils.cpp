#include "rendering_inicialization_utils.h"

#include "global_components/render_global_component.h"

#include "systems/render_clear_system.h"
#include "systems/render_present_system.h"
#include "systems/sprite_renderer_system.h"
#include "systems/gizmo_renderer_system.h"

#include "ecs/world.h"

#include "SDL_image.h"

namespace Engine
{
	static bool InitSDLRendering( RenderGlobalComponent& render )
	{
		if ( SDL_WasInit( SDL_INIT_VIDEO ) != 0 )
		{
			return false;
		}

		int32 result = SDL_InitSubSystem( SDL_INIT_VIDEO );
		if ( result != 0 )
		{
			LOG_ERROR( "Error while initializing SDL Video subsystem. Error code: %s", SDL_GetError() );
			return false;
		}

		result = SDL_CreateWindowAndRenderer( 512, 512, SDL_WINDOW_SHOWN, &render.window, &render.renderer );

		if ( result != 0 )
		{
			LOG_ERROR( "Error while initializing SDL window. Error code: %s", SDL_GetError() );
			return false;
		}

		result = SDL_SetRenderDrawColor( render.renderer, 255, 0, 0, 255 );
		if ( result != 0 )
		{
			LOG_ERROR( "Error while setting SDL render draw color. Error code: %s", SDL_GetError() );
			return false;
		}

		result = SDL_SetRelativeMouseMode( SDL_TRUE );
		if ( result != 0 )
		{
			LOG_ERROR( "Error while setting SDL mouse to relative mode. Error code: %s", SDL_GetError() );
			return false;
		}

		return true;
	}

	static bool AddRenderingSystems( ECS::World& world, RenderGlobalComponent& render )
	{
		ECS::SystemCoordinator* renderSystemCoordinator = new ECS::SystemCoordinator( ECS::ExecutionStage::RENDER );

		Engine::RenderClearSystem* renderClearSystem = new Engine::RenderClearSystem();
		renderSystemCoordinator->AddSystemToTail( renderClearSystem );

		SpriteRendererSystem* spriteRendererSystem = new SpriteRendererSystem( render.renderer );
		auto on_configure_sprite_renderer_callback =
		    std::bind( &SpriteRendererSystem::ConfigureSpriteRendererComponent, spriteRendererSystem,
		               std::placeholders::_1, std::placeholders::_2 );
		world.SubscribeToOnEntityConfigure( on_configure_sprite_renderer_callback );
		renderSystemCoordinator->AddSystemToTail( spriteRendererSystem );

		GizmoRendererSystem* gizmoRendererSystem = new GizmoRendererSystem();
		renderSystemCoordinator->AddSystemToTail( gizmoRendererSystem );

		RenderPresentSystem* renderPresentSystem = new RenderPresentSystem();
		renderSystemCoordinator->AddSystemToTail( renderPresentSystem );
		world.AddSystem( renderSystemCoordinator );
		world.SubscribeToOnEntityCreate( std::bind( &GizmoRendererSystem::AllocateGizmoRendererComponent,
		                                            gizmoRendererSystem, std::placeholders::_1 ) );
		world.SubscribeToOnEntityDestroy( std::bind( &GizmoRendererSystem::DeallocateGizmoRendererComponent,
		                                             gizmoRendererSystem, std::placeholders::_1 ) );

		return true;
	}

	bool AddRenderingToWorld( ECS::World& world )
	{
		RenderGlobalComponent& renderGlobalComponent = world.AddGlobalComponent< RenderGlobalComponent >();

		bool result = InitSDLRendering( renderGlobalComponent );
		if ( !result )
		{
			return false;
		}

		result = AddRenderingSystems( world, renderGlobalComponent );
		if ( !result )
		{
			return false;
		}

		return true;
	}

	static bool ShutdownRendering( Engine::RenderGlobalComponent& render )
	{
		if ( SDL_WasInit( SDL_INIT_VIDEO ) == 0 )
		{
			return false;
		}

		SDL_DestroyRenderer( render.renderer );
		render.renderer = nullptr;
		SDL_DestroyWindow( render.window );
		render.window = nullptr;

		SDL_QuitSubSystem( SDL_INIT_VIDEO );
		return true;
	}
} // namespace Engine
