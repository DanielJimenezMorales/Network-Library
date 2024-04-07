#pragma once
#include <cassert>

#include "Server.h"
#include "Client.h"
#include "Logger.h"
#include "Initializer.h"
#include "LogTestUtils.h"

//TODO There is a bug when connecting with high jitter, packet loss... It looks like the server gets full of connections for some reason and the connection is denied. Ans then it crash.
namespace Tests
{
    const unsigned int FIXED_FRAMES_PER_SECOND = 50;
    const float FIXED_FRAME_TARGET_DURATION = 1.0f / FIXED_FRAMES_PER_SECOND;

	class PeerConnectivityTests
	{
    private:
        void static SetUp()
        {
            NetLib::Initializer::Initialize();
        }

        void static TearDown()
        {
            NetLib::Initializer::Finalize();
        }

	public:
        bool static ExecuteAll()
        {
            LogTestUtils::LogTestResult(Test_ServerOnLocalPeerConnectDelegate_CheckItIsCalledOnlyOnce());
            LogTestUtils::LogTestResult(Test_ClientOnLocalPeerConnectDelegate_CheckItIsCalledOnlyOnce());
            LogTestUtils::LogTestResult(Test_ServerOnLocalPeerDisconnectDelegate_CheckItIsCalledOnlyOnce());
            LogTestUtils::LogTestResult(Test_ClientOnLocalPeerDisconnectDelegate_CheckItIsCalledOnlyOnce());
            LogTestUtils::LogTestResult(Test_ServerOnRemotePeerConnectDelegate_CheckItIsCalledOnlyOnce());
            LogTestUtils::LogTestResult(Test_ClientOnRemotePeerConnectDelegate_CheckItIsCalledOnlyOnce());
            LogTestUtils::LogTestResult(Test_ClientOnRemotePeerDisconnectDelegate_CheckItIsCalledOnlyOnceWhenServerStops());
            return true;
        }

        bool static Test_ServerOnLocalPeerConnectDelegate_CheckItIsCalledOnlyOnce()
        {
            LogTestUtils::LogTestName("Test_ServerOnLocalPeerConnectDelegate_CheckItIsCalledOnlyOnce");

            //Set up
            SetUp();

            //Arrange
            const unsigned int serverMaxConnections = 1;

            NetLib::Peer* serverPeer = new NetLib::Server(serverMaxConnections);

            int numberOfTimesCalled = 0;
            auto callback = [&numberOfTimesCalled]()
            {
                ++numberOfTimesCalled;
            };

            unsigned int subscriberId = 0;

            //Act
            subscriberId = serverPeer->SubscribeToOnLocalPeerConnect(callback);
            serverPeer->Start();
            serverPeer->Stop();
            serverPeer->UnsubscribeToOnPeerConnected(subscriberId);

            delete serverPeer;
            serverPeer = nullptr;

            //Assert
            assert(numberOfTimesCalled == 1);

            //Tear down
            TearDown();

            return true;
        }

        bool static Test_ClientOnLocalPeerConnectDelegate_CheckItIsCalledOnlyOnce()
        {
            LogTestUtils::LogTestName("Test_ClientOnLocalPeerConnectDelegate_CheckItIsCalledOnlyOnce");

            //Set up
            SetUp();

            //Arrange
            const float clientServerInactivityTimeout = 5;
            const unsigned int serverMaxConnections = 1;
            const float testTimeout = 2;

            NetLib::Peer* serverPeer = new NetLib::Server(serverMaxConnections);
            NetLib::Peer* clientPeer = new NetLib::Client(clientServerInactivityTimeout);

            int numberOfTimesCalled = 0;
            bool isRunning = true;

            auto callback = [&isRunning, &numberOfTimesCalled]()
            {
                isRunning = false;
                ++numberOfTimesCalled;
            };

            NetLib::TimeClock& timeClock = NetLib::TimeClock::GetInstance();
            double accumulator = 0.0;
            float testTimeLeft = testTimeout;

            unsigned int subscriberId = 0;

            //Act
            subscriberId = clientPeer->SubscribeToOnLocalPeerConnect(callback);
            serverPeer->Start();
            clientPeer->Start();

            while (isRunning)
            {
                timeClock.UpdateLocalTime();
                accumulator += timeClock.GetElapsedTimeSeconds();

                while (accumulator >= FIXED_FRAME_TARGET_DURATION)
                {
                    serverPeer->Tick(FIXED_FRAME_TARGET_DURATION);
                    clientPeer->Tick(FIXED_FRAME_TARGET_DURATION);

                    accumulator -= FIXED_FRAME_TARGET_DURATION;
                    testTimeLeft -= FIXED_FRAME_TARGET_DURATION;
                    if (testTimeLeft <= 0.f)
                    {
                        isRunning = false;
                    }
                }
            }

            serverPeer->Stop();
            clientPeer->Stop();

            clientPeer->UnsubscribeToOnPeerConnected(subscriberId);

            delete serverPeer;
            serverPeer = nullptr;
            delete clientPeer;
            clientPeer = nullptr;

            //Assert
            assert(numberOfTimesCalled == 1);

            //Tear down
            TearDown();

            return true;
        }

        bool static Test_ServerOnLocalPeerDisconnectDelegate_CheckItIsCalledOnlyOnce()
        {
            LogTestUtils::LogTestName("Test_ServerOnLocalPeerDisconnectDelegate_CheckItIsCalledOnlyOnce");

            //Set up
            SetUp();

            //Arrange
            const unsigned int serverMaxConnections = 1;

            NetLib::Peer* serverPeer = new NetLib::Server(serverMaxConnections);

            int numberOfTimesCalled = 0;
            auto callback = [&numberOfTimesCalled]()
            {
                ++numberOfTimesCalled;
            };

            unsigned int subscriberId = 0;

            //Act
            subscriberId = serverPeer->SubscribeToOnLocalPeerDisconnect(callback);
            serverPeer->Start();
            serverPeer->Stop();
            serverPeer->UnsubscribeToOnPeerDisconnected(subscriberId);

            delete serverPeer;
            serverPeer = nullptr;

            //Assert
            assert(numberOfTimesCalled == 1);

            //Tear down
            TearDown();

            return true;
        }

        bool static Test_ClientOnLocalPeerDisconnectDelegate_CheckItIsCalledOnlyOnce()
        {
            LogTestUtils::LogTestName("Test_ClientOnLocalPeerDisconnectDelegate_CheckItIsCalledOnlyOnce");

            //Set up
            SetUp();

            //Arrange
            const float clientServerInactivityTimeout = 5;
            const unsigned int serverMaxConnections = 1;
            const float testTimeout = 2;

            NetLib::Peer* serverPeer = new NetLib::Server(serverMaxConnections);
            NetLib::Peer* clientPeer = new NetLib::Client(clientServerInactivityTimeout);

            int numberOfTimesCalled = 0;
            bool isRunning = true;

            auto callback = [&isRunning, &numberOfTimesCalled]()
            {
                isRunning = false;
                ++numberOfTimesCalled;
            };

            NetLib::TimeClock& timeClock = NetLib::TimeClock::GetInstance();
            double accumulator = 0.0;
            float testTimeLeft = testTimeout;
            float disconnectClientTimeout = 1.f;
            bool isAlreadyDisconnected = false;

            unsigned int subscriberId = 0;

            //Act
            subscriberId = clientPeer->SubscribeToOnLocalPeerConnect(callback);
            serverPeer->Start();
            clientPeer->Start();

            while (isRunning)
            {
                timeClock.UpdateLocalTime();
                accumulator += timeClock.GetElapsedTimeSeconds();

                while (accumulator >= FIXED_FRAME_TARGET_DURATION)
                {
                    serverPeer->Tick(FIXED_FRAME_TARGET_DURATION);
                    clientPeer->Tick(FIXED_FRAME_TARGET_DURATION);

                    accumulator -= FIXED_FRAME_TARGET_DURATION;
                    disconnectClientTimeout -= FIXED_FRAME_TARGET_DURATION;
                    if (disconnectClientTimeout <= 0.f && !isAlreadyDisconnected)
                    {
                        clientPeer->Stop();
                        isAlreadyDisconnected = true;
                    }

                    testTimeLeft -= FIXED_FRAME_TARGET_DURATION;
                    if (testTimeLeft <= 0.f)
                    {
                        isRunning = false;
                    }
                }
            }

            serverPeer->Stop();

            clientPeer->UnsubscribeToOnPeerConnected(subscriberId);

            delete serverPeer;
            serverPeer = nullptr;
            delete clientPeer;
            clientPeer = nullptr;

            //Assert
            assert(numberOfTimesCalled == 1);

            //Tear down
            TearDown();

            return true;
        }

        bool static Test_ServerOnRemotePeerConnectDelegate_CheckItIsCalledOnlyOnce()
        {
            LogTestUtils::LogTestName("Test_ServerOnRemotePeerConnectDelegate_CheckItIsCalledOnlyOnce");

            //Set up
            SetUp();

            //Arrange
            const float clientServerInactivityTimeout = 5;
            const unsigned int serverMaxConnections = 1;
            const float testTimeout = 2;

            NetLib::Peer* serverPeer = new NetLib::Server(serverMaxConnections);
            NetLib::Peer* clientPeer = new NetLib::Client(clientServerInactivityTimeout);

            int numberOfTimesCalled = 0;
            bool isRunning = true;

            auto callback = [&isRunning, &numberOfTimesCalled]()
            {
                isRunning = false;
                ++numberOfTimesCalled;
            };

            NetLib::TimeClock& timeClock = NetLib::TimeClock::GetInstance();
            double accumulator = 0.0;
            float testTimeLeft = testTimeout;

            unsigned int subscriberId = 0;

            //Act
            subscriberId = serverPeer->SubscribeToOnRemotePeerConnect(callback);
            serverPeer->Start();
            clientPeer->Start();

            while (isRunning)
            {
                timeClock.UpdateLocalTime();
                accumulator += timeClock.GetElapsedTimeSeconds();

                while (accumulator >= FIXED_FRAME_TARGET_DURATION)
                {
                    serverPeer->Tick(FIXED_FRAME_TARGET_DURATION);
                    clientPeer->Tick(FIXED_FRAME_TARGET_DURATION);

                    accumulator -= FIXED_FRAME_TARGET_DURATION;
                    testTimeLeft -= FIXED_FRAME_TARGET_DURATION;
                    if (testTimeLeft <= 0.f)
                    {
                        isRunning = false;
                    }
                }
            }

            serverPeer->Stop();
            clientPeer->Stop();
            serverPeer->UnsubscribeToOnRemotePeerConnect(subscriberId);

            delete serverPeer;
            serverPeer = nullptr;
            delete clientPeer;
            clientPeer = nullptr;

            //Assert
            assert(numberOfTimesCalled == 1);

            //Tear down
            TearDown();

            return true;
        }

        bool static Test_ClientOnRemotePeerConnectDelegate_CheckItIsCalledOnlyOnce()
        {
            LogTestUtils::LogTestName("Test_ClientOnRemotePeerConnectDelegate_CheckItIsCalledOnlyOnce");

            //Set up
            SetUp();

            //Arrange
            const float clientServerInactivityTimeout = 5;
            const unsigned int serverMaxConnections = 1;
            const float testTimeout = 2;

            NetLib::Peer* serverPeer = new NetLib::Server(serverMaxConnections);
            NetLib::Peer* clientPeer = new NetLib::Client(clientServerInactivityTimeout);

            int numberOfTimesCalled = 0;
            bool isRunning = true;

            auto callback = [&isRunning, &numberOfTimesCalled]()
            {
                isRunning = false;
                ++numberOfTimesCalled;
            };

            NetLib::TimeClock& timeClock = NetLib::TimeClock::GetInstance();
            double accumulator = 0.0;
            float testTimeLeft = testTimeout;

            unsigned int subscriberId = 0;

            //Act
            subscriberId = clientPeer->SubscribeToOnRemotePeerConnect(callback);
            serverPeer->Start();
            clientPeer->Start();

            while (isRunning)
            {
                timeClock.UpdateLocalTime();
                accumulator += timeClock.GetElapsedTimeSeconds();

                while (accumulator >= FIXED_FRAME_TARGET_DURATION)
                {
                    serverPeer->Tick(FIXED_FRAME_TARGET_DURATION);
                    clientPeer->Tick(FIXED_FRAME_TARGET_DURATION);

                    accumulator -= FIXED_FRAME_TARGET_DURATION;
                    testTimeLeft -= FIXED_FRAME_TARGET_DURATION;
                    if (testTimeLeft <= 0.f)
                    {
                        isRunning = false;
                    }
                }
            }

            serverPeer->Stop();
            clientPeer->Stop();
            clientPeer->UnsubscribeToOnRemotePeerConnect(subscriberId);

            delete serverPeer;
            serverPeer = nullptr;
            delete clientPeer;
            clientPeer = nullptr;

            //Assert
            assert(numberOfTimesCalled == 1);

            //Tear down
            TearDown();

            return true;
        }

        bool static Test_ClientOnRemotePeerDisconnectDelegate_CheckItIsCalledOnlyOnceWhenServerStops()
        {
            LogTestUtils::LogTestName("Test_ClientOnRemotePeerDisconnectDelegate_CheckItIsCalledOnlyOnceWhenServerStops");

            //Set up
            SetUp();

            //Arrange
            const float clientServerInactivityTimeout = 5;
            const unsigned int serverMaxConnections = 1;
            const float testTimeout = 2;

            NetLib::Peer* serverPeer = new NetLib::Server(serverMaxConnections);
            NetLib::Peer* clientPeer = new NetLib::Client(clientServerInactivityTimeout);

            int numberOfTimesCalled = 0;
            bool isRunning = true;

            auto callback = [&isRunning, &numberOfTimesCalled]()
            {
                isRunning = false;
                ++numberOfTimesCalled;
            };

            NetLib::TimeClock& timeClock = NetLib::TimeClock::GetInstance();
            double accumulator = 0.0;
            float testTimeLeft = testTimeout;
            float disconnectServerTimeout = 0.6f;
            bool isServerDisconnected = false;

            unsigned int subscriberId = 0;

            //Act
            subscriberId = clientPeer->SubscribeToOnRemotePeerDisconnect(callback);
            serverPeer->Start();
            clientPeer->Start();

            while (isRunning)
            {
                timeClock.UpdateLocalTime();
                accumulator += timeClock.GetElapsedTimeSeconds();

                while (accumulator >= FIXED_FRAME_TARGET_DURATION)
                {
                    serverPeer->Tick(FIXED_FRAME_TARGET_DURATION);
                    clientPeer->Tick(FIXED_FRAME_TARGET_DURATION);

                    accumulator -= FIXED_FRAME_TARGET_DURATION;
                    disconnectServerTimeout -= FIXED_FRAME_TARGET_DURATION;
                    if (disconnectServerTimeout <= 0.f && !isServerDisconnected)
                    {
                        serverPeer->Stop();
                        isServerDisconnected = true;
                    }

                    testTimeLeft -= FIXED_FRAME_TARGET_DURATION;
                    if (testTimeLeft <= 0.f)
                    {
                        isRunning = false;
                    }
                }
            }

            clientPeer->Stop();
            clientPeer->UnsubscribeToOnRemotePeerDisconnect(subscriberId);

            delete serverPeer;
            serverPeer = nullptr;
            delete clientPeer;
            clientPeer = nullptr;

            //Assert
            assert(numberOfTimesCalled == 1);

            //Tear down
            TearDown();

            return true;
        }
	};
}
