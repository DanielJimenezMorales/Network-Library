// network-library.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <chrono>

#include "Server.h"
#include "Client.h"
#include "Logger.h"
#include "MessageFactory.h"

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT "27015"
#define DEFAULT_BUFFER_LENGTH 512

const unsigned int FIXED_FRAMES_PER_SECOND = 50;
const float FIXED_FRAME_TARGET_DURATION = 1.0f / FIXED_FRAMES_PER_SECOND;
bool isRunning = true;

//#pragma comment(lib, "Ws2_32.lib") //Added to Properties/Linker/Input/Additional Dependencies
int main()
{
    std::cout << "Select:\nServer: 0\nClient: 1\n";

    int clientOrServer;
    std::cin >> clientOrServer;

    MessageFactory::GetInstance(1);

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
    std::chrono::system_clock::time_point lastTime = std::chrono::system_clock::now();
    double elapsedTime = 0.0;
    double accumulator = 0.0;

    while (isRunning)
    {
        std::chrono::system_clock::time_point current = std::chrono::system_clock::now();
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

    return EXIT_SUCCESS;
}


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

/*
* Clase Cliente:
* - Solo va a conectarse a un servidor, y mostrar por consola los mensajes que le lleguen del servidor.
*
* Clase Server:
* - Varios clientes van a poder conectarse a él (Hasta un límite)
* - Por cada tick, el servidor enviará un mensaje a cada cliente.
* -
*/