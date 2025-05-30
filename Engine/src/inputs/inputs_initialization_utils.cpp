#include "inputs_initialization_utils.h"

#include "global_components/input_handler_global_component.h"

#include "systems/input_handler_system.h"

#include "ecs/world.h"

#include <SDL.h>

namespace Engine
{
	static bool InitSDLInput()
	{
		if ( SDL_WasInit( SDL_INIT_EVENTS ) != 0 )
		{
			return false;
		}

		const int32 result = SDL_InitSubSystem( SDL_INIT_EVENTS );
		if ( result != 0 )
		{
			LOG_ERROR( "Error while initializing SDL events subsystem. Error code: %s", SDL_GetError() );
			return false;
		}

		return true;
	}

	static bool AddSystemsToWorld( ECS::World& world )
	{
		ECS::SystemCoordinator* inputHandlerSystemCoordinator =
		    new ECS::SystemCoordinator( ECS::ExecutionStage::INPUT_HANDLING );

		InputHandlerSystem* inputHandlerSystem = new InputHandlerSystem();
		inputHandlerSystemCoordinator->AddSystemToTail( inputHandlerSystem );

		world.AddSystem( inputHandlerSystemCoordinator );

		return true;
	}

	bool AddInputsToWorld( ECS::World& world )
	{
		bool result = InitSDLInput();
		if ( !result )
		{
			return false;
		}

		world.AddGlobalComponent< InputHandlerGlobalComponent >();

		result = AddSystemsToWorld( world );
		if ( !result )
		{
			return false;
		}

		return true;
	}
} // namespace Engine
