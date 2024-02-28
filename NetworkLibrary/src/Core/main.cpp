// network-library.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <chrono>

#include "Server.h"
#include "Client.h"
#include "Logger.h"
#include "MessageFactory.h"
#include "TimeClock.h"

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT "27015"
#define DEFAULT_BUFFER_LENGTH 512

const unsigned int FIXED_FRAMES_PER_SECOND = 50;
const float FIXED_FRAME_TARGET_DURATION = 1.0f / FIXED_FRAMES_PER_SECOND;
bool isRunning = true;

//TODO Add a maximum size to each packet and add as many messages as possible until reaching the max size

//#pragma comment(lib, "Ws2_32.lib") //Added to Properties/Linker/Input/Additional Dependencies
int main()
{
    std::cout << "Select:\nServer: 0\nClient: 1\n";

    int clientOrServer;
    std::cin >> clientOrServer;

    MessageFactory::CreateInstance(1);
    TimeClock::CreateInstance();

    Peer* peer = nullptr;

    if (clientOrServer == 0)
    {
        peer = new Server(2);
    }
    else if (clientOrServer == 1)
    {
        peer = new Client(5);
    }

    bool result = peer->Start();
    if (!result)
    {
        LOG_ERROR("Peer startup failed");
    }

    //GAMELOOP BEGIN
    double time = 0.0;
    std::chrono::steady_clock::time_point lastTime = std::chrono::steady_clock::now();
    double elapsedTime = 0.0;
    double accumulator = 0.0;

    while (isRunning)
    {
        std::chrono::steady_clock::time_point current = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsedTimeInSeconds = current - lastTime;
        time += elapsedTimeInSeconds.count();
        accumulator += elapsedTimeInSeconds.count();

        //game.HandleEvents();

        while (accumulator >= FIXED_FRAME_TARGET_DURATION)
        {
            peer->Tick(FIXED_FRAME_TARGET_DURATION);

            accumulator -= FIXED_FRAME_TARGET_DURATION;
        }

        lastTime = current;
    }
    //GAMELOOP END

    result = peer->Stop();
    if (!result)
    {
        LOG_ERROR("Peer stop failed");
    }

    delete peer;
    peer = nullptr;

    MessageFactory::DeleteInstance();
    TimeClock::DeleteInstance();

    return EXIT_SUCCESS;
}