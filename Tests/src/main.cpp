// network-library.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <chrono>
#include <sstream>
#include <cassert>

#include "Server.h"
#include "Client.h"
#include "Logger.h"
#include "Initializer.h"

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT "27015"
#define DEFAULT_BUFFER_LENGTH 512

const unsigned int FIXED_FRAMES_PER_SECOND = 50;
const float FIXED_FRAME_TARGET_DURATION = 1.0f / FIXED_FRAMES_PER_SECOND;
bool isRunning = true;

void LogTestName(const std::string& name)
{
    std::stringstream ss;
    ss << "\n**********************************************\n" << name << "\n" << "**********************************************";
    Common::LOG_INFO(ss.str());
}

void LogTestResult(bool correct)
{
    std::string text;
    if (correct)
    {
        text = "CORRECT";
    }
    else
    {
        text = "ERROR";
    }

    std::stringstream ss;
    ss << "\n**********************************************\n" << text << "\n" << "**********************************************";
    Common::LOG_INFO(ss.str());
}

void SetUp()
{
    NetLib::Initializer::Initialize();
}

void TearDown()
{
    NetLib::Initializer::Finalize();
}

bool TestServerOnLocalPeerConnectDelegate()
{
    LogTestName("TestServerOnLocalPeerConnectDelegate");

    //Set up
    SetUp();

    //Arrange
    const float clientServerInactivityTimeout = 5;
    const unsigned int serverMaxConnections = 1;

    NetLib::Peer* serverPeer = new NetLib::Server(serverMaxConnections);

    bool isOnLocalPeerConnectDelegateCalled = false;
    auto callback = [&isOnLocalPeerConnectDelegateCalled]()
    {
        isOnLocalPeerConnectDelegateCalled = true;
    };
    
    //Act
    serverPeer->SubscribeToOnLocalPeerConnect(callback);
    serverPeer->Start();
    serverPeer->Stop();

    //Assert
    assert(isOnLocalPeerConnectDelegateCalled);

    //Tear down
    TearDown();

    return true;
}

//#pragma comment(lib, "Ws2_32.lib") //Added to Properties/Linker/Input/Additional Dependencies
int main()
{
    LogTestResult(TestServerOnLocalPeerConnectDelegate());
    return 0;
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
        Common::LOG_ERROR("Peer startup failed");
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

        if (clientOrServer == 1 && timeClock.GetLocalTimeSeconds() > 5.0f)
        {
            peer->Stop();
            isRunning = false;
        }
    }
    //GAMELOOP END

    result = peer->Stop();
    if (!result)
    {
        Common::LOG_ERROR("Peer stop failed");
    }

    delete peer;
    peer = nullptr;

    NetLib::TimeClock::DeleteInstance();

    return EXIT_SUCCESS;
}
