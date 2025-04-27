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
#include "CircleBounds2D.h"

int main()
{
	/*
	TransformComponent transform;
	transform.SetPosition( Vec2f( 10, 10 ) );
	CircleBounds2D circle_radius( 5 );
	Ray ray;
	ray.origin = Vec2f( 0, 0 );
	ray.direction = Vec2f( 1, 1 );
	ray.direction.Normalize();
	ray.maxDistance = 10;
	RaycastResult ray_result;
	bool result = PerformRaycastAgainstSphere( ray, transform, circle_radius, ray_result );
	return EXIT_SUCCESS;*/

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
