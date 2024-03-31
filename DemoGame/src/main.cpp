// network-library.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define SDL_MAIN_HANDLED
#include <iostream>
#include <chrono>
#include <sstream>
#include <SDL.h>

#include "Server.h"
#include "Client.h"
#include "Logger.h"
#include "TimeClock.h"
#include "Game.h"

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT "27015"
#define DEFAULT_BUFFER_LENGTH 512

//TODO Add a maximum size to each packet and add as many messages as possible until reaching the max size

//#pragma comment(lib, "Ws2_32.lib") //Added to Properties/Linker/Input/Additional Dependencies
int main()
{
    Game game;
    if (!game.Init())
    {
        return EXIT_FAILURE;
    }

    game.GameLoop();
    game.Release();

    return EXIT_SUCCESS;
}
