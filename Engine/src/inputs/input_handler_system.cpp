#include "input_handler_system.h"

#include <SDL.h>

#include "inputs/input_handler_global_component.h"
#include "inputs/i_input_controller.h"
#include "inputs/i_cursor.h"

#include "ecs/world.h"

namespace Engine
{
	InputHandlerSystem::InputHandlerSystem()
	    : ECS::ISimpleSystem()
	    , _isGameRunning( true )
	{
	}

	static void PreHandleEvents( InputHandlerGlobalComponent& input_handler )
	{
		auto controllers_it = input_handler.controllers.begin();
		for ( ; controllers_it != input_handler.controllers.end(); ++controllers_it )
		{
			controllers_it->second->ResetEvents();
		}

		auto cursors_it = input_handler.cursors.begin();
		for ( ; cursors_it != input_handler.cursors.end(); ++cursors_it )
		{
			cursors_it->second->ResetEvents();
		}
	}

	static void HandleEvent( InputHandlerGlobalComponent& input_handler, const SDL_Event& event )
	{
		auto controllers_it = input_handler.controllers.begin();
		for ( ; controllers_it != input_handler.controllers.end(); ++controllers_it )
		{
			controllers_it->second->HandleEvent( event );
		}

		auto cursors_it = input_handler.cursors.begin();
		for ( ; cursors_it != input_handler.cursors.end(); ++cursors_it )
		{
			cursors_it->second->HandleEvent( event );
		}
	}

	static void PostHandleEvents( InputHandlerGlobalComponent& input_handler )
	{
		auto controllers_it = input_handler.controllers.begin();
		for ( ; controllers_it != input_handler.controllers.end(); ++controllers_it )
		{
			controllers_it->second->UpdateUnhandledButtons();
		}

		auto cursors_it = input_handler.cursors.begin();
		for ( ; cursors_it != input_handler.cursors.end(); ++cursors_it )
		{
			cursors_it->second->UpdateUnhandledButtons();
		}
	}

	void Engine::InputHandlerSystem::Execute( ECS::World& world, float32 elapsed_time )
	{
		InputHandlerGlobalComponent& inputHandlerGlobalComponent =
		    world.GetGlobalComponent< InputHandlerGlobalComponent >();

		PreHandleEvents( inputHandlerGlobalComponent );

		SDL_Event ev;
		while ( SDL_PollEvent( &ev ) )
		{
			if ( ev.type == SDL_QUIT )
			{
				// TODO Make something to actually stop the program
				LOG_INFO( "Received quit event" );
				_isGameRunning = false;
				break;
			}

			HandleEvent( inputHandlerGlobalComponent, ev );
		}

		PostHandleEvents( inputHandlerGlobalComponent );
	}
} // namespace Engine
