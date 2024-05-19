#define SDL_MAIN_HANDLED
#include <iostream>

#include "Game.h"

int main()
{
    Game game;
    if (!game.Init())
    {
        return EXIT_FAILURE;
    }

    game.GameLoop();
    if (!game.Release())
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
