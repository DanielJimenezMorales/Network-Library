#define SDL_MAIN_HANDLED
#include <iostream>

#include "Game.h"

#ifdef _WIN32
	#include <windows.h>
	#include <filesystem>
// This is only for windows to enable ANSI colors on windows console
void EnableVTMode()
{
	HANDLE hOut = GetStdHandle( STD_OUTPUT_HANDLE );
	if ( hOut == INVALID_HANDLE_VALUE )
	{
		std::cerr << "Error: Unable to get console handle" << std::endl;
		return;
	}

	DWORD dwMode = 0;
	if ( !GetConsoleMode( hOut, &dwMode ) )
	{
		std::cerr << "Error: Unable to get console mode" << std::endl;
		return;
	}

	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	if ( !SetConsoleMode( hOut, dwMode ) )
	{
		std::cerr << "Error: Unable to set console mode" << std::endl;
		return;
	}
}
#endif

#include "raycaster.h"
#include "components/transform_component.h"

#include "client/client_world_initializer.h"
#include "server/server_world_initializer.h"

int main()
{
#ifdef _WIN32
	EnableVTMode();
#endif

	LOG_INFO( "Select:" );
	LOG_INFO( "Server: 0" );
	LOG_INFO( "Client: 1" );

	int32 clientOrServer;
	std::cin >> clientOrServer;

	Engine::IWorldInitializer* worldInitializer = nullptr;
	if ( clientOrServer == 0 )
	{
		worldInitializer = new ServerWorldInitializer();
	}
	else if ( clientOrServer == 1 )
	{
		worldInitializer = new ClientWorldInitializer();
	}

	Game game;
	if ( !game.Init( worldInitializer ) )
	{
		return EXIT_FAILURE;
	}

	game.GameLoop();
	if ( !game.Release() )
	{
		return EXIT_FAILURE;
	}

	delete worldInitializer;

	return EXIT_SUCCESS;
}
