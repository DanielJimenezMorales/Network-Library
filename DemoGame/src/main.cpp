// network-library.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <chrono>
#include <sstream>

#include "Server.h"
#include "Client.h"
#include "Logger.h"
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

    NetLib::TimeClock::CreateInstance();

    NetLib::Peer* peer = nullptr;

    if (clientOrServer == 0)
    {
        peer = new NetLib::Server(2);
    }
    else if (clientOrServer == 1)
    {
        peer = new NetLib::Client(5);
    }

    bool result = peer->Start();
    if (!result)
    {
        NetLib::LOG_ERROR("Peer startup failed");
    }

    //GAMELOOP BEGIN
    NetLib::TimeClock& timeClock = NetLib::TimeClock::GetInstance();
    double accumulator = 0.0;

    while (isRunning)
    {
        timeClock.UpdateLocalTime();
        accumulator += timeClock.GetElapsedTimeSeconds();

        //game.HandleEvents();

        while (accumulator >= FIXED_FRAME_TARGET_DURATION)
        {
            peer->Tick(FIXED_FRAME_TARGET_DURATION);

            accumulator -= FIXED_FRAME_TARGET_DURATION;
        }
    }
    //GAMELOOP END

    result = peer->Stop();
    if (!result)
    {
        NetLib::LOG_ERROR("Peer stop failed");
    }

    delete peer;
    peer = nullptr;

    NetLib::TimeClock::DeleteInstance();

    return EXIT_SUCCESS;
}
