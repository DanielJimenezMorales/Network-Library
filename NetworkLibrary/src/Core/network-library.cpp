// network-library.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <winsock2.h>
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

    WSADATA wsaData;
    int iResult;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);//Init WS. You need to pass it the version (1.0, 1.1, 2.2...) and a pointer to WSADATA which contains info about the WS impl.
    if (iResult != 0)
    {
        LOG_ERROR("WSAStartup failed: " + iResult);
        return EXIT_FAILURE;
    }

    MessageFactory::GetInstance(1);

    Server* server = nullptr;
    Client* client = nullptr;

    if (clientOrServer == 0)
    {
        server = new Server(2);
        if (!server->Start())
        {
            LOG_ERROR("Server startup failed");
        }
    }
    else if (clientOrServer == 1)
    {
        client = new Client(5);
        bool clientStartUpResult = client->Start();
        if (!clientStartUpResult)
        {
            LOG_ERROR("Client startup failed");
        }
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
            if (clientOrServer == 0)
            {
                server->Tick(FIXED_FRAME_TARGET_DURATION);
            }
            else if (clientOrServer == 1)
            {
                client->Tick(FIXED_FRAME_TARGET_DURATION);
            }

            accumulator -= FIXED_FRAME_TARGET_DURATION;
        }

        lastTime = current;
    }
    //GAMELOOP END

    if (clientOrServer == 0)
    {
        int serverStopResult = server->Stop();
        if (serverStopResult != 0)
        {
            LOG_ERROR("Server stop failed with error " + serverStopResult);
        }
        delete server;
        server = nullptr;
    }
    else if (clientOrServer == 1)
    {
        int clientStopResult = client->Stop();
        if (clientStopResult != 0)
        {
            LOG_ERROR("Client stop failed with error " + clientStopResult);
        }
        delete client;
        client = nullptr;
    }

    WSACleanup();
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