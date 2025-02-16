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

int main()
{
#ifdef _WIN32
	EnableVTMode();
#endif

	Game game;
	if ( !game.Init() )
	{
		return EXIT_FAILURE;
	}

	game.GameLoop();
	if ( !game.Release() )
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
