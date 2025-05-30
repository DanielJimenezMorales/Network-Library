#include "server_dummy_input_handler_system.h"

#include <SDL_events.h>

ServerDummyInputHandlerSystem::ServerDummyInputHandlerSystem()
    : Engine::ECS::ISimpleSystem()
{
}

void ServerDummyInputHandlerSystem::Execute( Engine::ECS::World& world, float32 elapsed_time )
{
	// Poll events to prevent SDL from freezing
	SDL_Event ev;
	SDL_PollEvent( &ev );
}
