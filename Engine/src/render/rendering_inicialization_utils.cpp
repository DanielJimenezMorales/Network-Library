#include "rendering_inicialization_utils.h"

#include "render/render_global_component.h"
#include "render/render_clear_system.h"
#include "render/render_present_system.h"
#include "render/sprite_renderer_system.h"
#include "render/gizmo_renderer_system.h"
#include "render/texture_asset_loader.h"
#include "render/color.h"

#include "asset_manager/asset_manager.h"

#include "ecs/world.h"

#include "game.h"

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

		result = SDL_SetRelativeMouseMode( SDL_TRUE );
		if ( result != 0 )
		{
			LOG_ERROR( "Error while setting SDL mouse to relative mode. Error code: %s", SDL_GetError() );
			return false;
		}

		return true;
	}

	static bool AddRenderingAssetmanagement( Game& game, RenderGlobalComponent& render )
	{
		AssetManager& assetManager = game.GetAssetManager();
		bool result = assetManager.RegisterAsset( AssetType::TEXTURE, new TextureAssetLoader( render.renderer ) );
		return result;
	}

	static bool AddRenderingSystems( Game& game, RenderGlobalComponent& render )
	{
		ECS::World& world = game.GetActiveWorld();

		ECS::SystemCoordinator* renderSystemCoordinator = new ECS::SystemCoordinator( ECS::ExecutionStage::RENDER );

		Engine::RenderClearSystem* renderClearSystem = new Engine::RenderClearSystem();
		renderSystemCoordinator->AddSystemToTail( renderClearSystem );

		SpriteRendererSystem* spriteRendererSystem =
		    new SpriteRendererSystem( render.renderer, &game.GetAssetManager() );
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

	bool AddRenderingToWorld( Game& game )
	{
		ECS::World& world = game.GetActiveWorld();
		RenderGlobalComponent& renderGlobalComponent = world.AddGlobalComponent< RenderGlobalComponent >();

		bool result = InitSDLRendering( renderGlobalComponent );
		if ( !result )
		{
			return false;
		}

		renderGlobalComponent.clearColor = Color::RED();

		result = AddRenderingAssetmanagement( game, renderGlobalComponent );
		if ( !result )
		{
			return false;
		}

		result = AddRenderingSystems( game, renderGlobalComponent );
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
