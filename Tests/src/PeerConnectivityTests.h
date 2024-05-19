#pragma once
#include <cassert>
#include <chrono>
#include <thread>

#include "Server.h"
#include "Client.h"
#include "Initializer.h"
#include "LogTestUtils.h"

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

        void static TickPeer(NetLib::Peer& peer, float elapsedTime)
        {
            peer.PreTick();
            peer.Tick(elapsedTime);
        }

	public:
        bool static ExecuteAll()
        {
            //Set a small break between different tests in order to avoid receiving messages from the last test
            std::chrono::milliseconds duration(500);

            //Test local peer connection
            LogTestUtils::LogTestResult(Test_ServerOnLocalPeerConnectDelegate_CheckItIsCalledOnlyOnce());
            std::this_thread::sleep_for(duration);

            LogTestUtils::LogTestResult(Test_ClientOnLocalPeerConnectDelegate_CheckItIsCalledOnlyOnce());
            std::this_thread::sleep_for(duration);

            //Test local peer disconnection
            LogTestUtils::LogTestResult(Test_ServerOnLocalPeerDisconnectDelegate_CheckItIsCalledOnlyOnce());
            std::this_thread::sleep_for(duration);

            LogTestUtils::LogTestResult(Test_ClientOnLocalPeerDisconnectDelegate_CheckItIsCalledOnlyOnce());
            std::this_thread::sleep_for(duration);

            LogTestUtils::LogTestResult(Test_ClientOnLocalPeerDisconnectDelegate_CheckItIsCalledOnlyOnceWhenServerStops());
            std::this_thread::sleep_for(duration);

            LogTestUtils::LogTestResult(Test_ClientOnLocalPeerDisconnectDelegate_CheckItIsCalledOnlyOnceWhenServerDenyConnectionDueToItIsFull());
            std::this_thread::sleep_for(duration);

            //Test remote peer connection
            LogTestUtils::LogTestResult(Test_ServerOnRemotePeerConnectDelegate_CheckItIsCalledOnlyOnce());
            std::this_thread::sleep_for(duration);

            LogTestUtils::LogTestResult(Test_ClientOnRemotePeerConnectDelegate_CheckItIsCalledOnlyOnce());
            std::this_thread::sleep_for(duration);

            //Test remote peer disconnection
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
            auto connectionStateAfterStart = serverPeer->GetConnectionState();
            serverPeer->Stop();
            serverPeer->UnsubscribeToOnPeerConnected(subscriberId);

            delete serverPeer;
            serverPeer = nullptr;

            //Assert
            assert(numberOfTimesCalled == 1);
            assert(connectionStateAfterStart == NetLib::PeerConnectionState::PCS_Connected);

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
            NetLib::PeerConnectionState clientPeerConnectionStateAfterOnConnect = NetLib::PeerConnectionState::PCS_Disconnected;

            auto callback = [&isRunning, &numberOfTimesCalled, &clientPeerConnectionStateAfterOnConnect, &clientPeer]()
            {
                isRunning = false;
                ++numberOfTimesCalled;
                clientPeerConnectionStateAfterOnConnect = clientPeer->GetConnectionState();
            };

            NetLib::TimeClock& timeClock = NetLib::TimeClock::GetInstance();
            double accumulator = 0.0;
            float testTimeLeft = testTimeout;

            unsigned int subscriberId = 0;

            //Act
            subscriberId = clientPeer->SubscribeToOnLocalPeerConnect(callback);
            serverPeer->Start();
            clientPeer->Start();
            auto clientPeerConnectionStateAfterStart = clientPeer->GetConnectionState();

            while (isRunning)
            {
                timeClock.UpdateLocalTime();
                accumulator += timeClock.GetElapsedTimeSeconds();

                while (accumulator >= FIXED_FRAME_TARGET_DURATION)
                {
                    TickPeer(*serverPeer, FIXED_FRAME_TARGET_DURATION);
                    TickPeer(*clientPeer, FIXED_FRAME_TARGET_DURATION);

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
            assert(clientPeerConnectionStateAfterStart == NetLib::PeerConnectionState::PCS_Connecting);
            assert(clientPeerConnectionStateAfterOnConnect == NetLib::PeerConnectionState::PCS_Connected);

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
            NetLib::PeerConnectionState serverConnectionStateAfterStop = NetLib::PeerConnectionState::PCS_Connected;
            NetLib::ConnectionFailedReasonType disconnectionReason = NetLib::ConnectionFailedReasonType::CFR_UNKNOWN;

            int numberOfTimesCalled = 0;
            auto callback = [&numberOfTimesCalled, &serverConnectionStateAfterStop, &serverPeer, &disconnectionReason](NetLib::ConnectionFailedReasonType reason)
            {
                ++numberOfTimesCalled;
                serverConnectionStateAfterStop = serverPeer->GetConnectionState();
                disconnectionReason = reason;
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
            assert(serverConnectionStateAfterStop == NetLib::PeerConnectionState::PCS_Disconnected);
            assert(disconnectionReason == NetLib::ConnectionFailedReasonType::CFR_PEER_SHUT_DOWN);

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
            NetLib::PeerConnectionState clientConnectionStateAfterStop = NetLib::PeerConnectionState::PCS_Connected;
            NetLib::ConnectionFailedReasonType disconnectionReason = NetLib::ConnectionFailedReasonType::CFR_UNKNOWN;

            auto callback = [&isRunning, &numberOfTimesCalled, &clientConnectionStateAfterStop, &clientPeer, &disconnectionReason](NetLib::ConnectionFailedReasonType reason)
            {
                isRunning = false;
                ++numberOfTimesCalled;
                clientConnectionStateAfterStop = clientPeer->GetConnectionState();
                disconnectionReason = reason;
            };

            NetLib::TimeClock& timeClock = NetLib::TimeClock::GetInstance();
            double accumulator = 0.0;
            float testTimeLeft = testTimeout;
            float disconnectClientTimeout = 1.f;
            bool isAlreadyDisconnected = false;

            unsigned int subscriberId = 0;

            //Act
            subscriberId = clientPeer->SubscribeToOnLocalPeerDisconnect(callback);
            serverPeer->Start();
            clientPeer->Start();

            while (isRunning)
            {
                timeClock.UpdateLocalTime();
                accumulator += timeClock.GetElapsedTimeSeconds();

                while (accumulator >= FIXED_FRAME_TARGET_DURATION)
                {
                    TickPeer(*serverPeer, FIXED_FRAME_TARGET_DURATION);
                    TickPeer(*clientPeer, FIXED_FRAME_TARGET_DURATION);

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

            clientPeer->UnsubscribeToOnPeerDisconnected(subscriberId);

            delete serverPeer;
            serverPeer = nullptr;
            delete clientPeer;
            clientPeer = nullptr;

            //Assert
            assert(numberOfTimesCalled == 1);
            assert(clientConnectionStateAfterStop == NetLib::PeerConnectionState::PCS_Disconnected);
            assert(disconnectionReason == NetLib::ConnectionFailedReasonType::CFR_PEER_SHUT_DOWN);

            //Tear down
            TearDown();

            return true;
        }

        bool static Test_ClientOnLocalPeerDisconnectDelegate_CheckItIsCalledOnlyOnceWhenServerStops()
        {
            LogTestUtils::LogTestName("Test_ClientOnLocalPeerDisconnectDelegate_CheckItIsCalledOnlyOnceWhenServerStops");

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
            NetLib::PeerConnectionState clientConnectionStateAfterStop = NetLib::PeerConnectionState::PCS_Connected;
            NetLib::ConnectionFailedReasonType disconnectionReason = NetLib::ConnectionFailedReasonType::CFR_UNKNOWN;

            auto callback = [&isRunning, &numberOfTimesCalled, &clientConnectionStateAfterStop, &clientPeer, &disconnectionReason](NetLib::ConnectionFailedReasonType reason)
            {
                isRunning = false;
                ++numberOfTimesCalled;
                clientConnectionStateAfterStop = clientPeer->GetConnectionState();
                disconnectionReason = reason;
            };

            NetLib::TimeClock& timeClock = NetLib::TimeClock::GetInstance();
            double accumulator = 0.0;
            float testTimeLeft = testTimeout;
            float disconnectServerTimeout = 1.f;
            bool isAlreadyDisconnected = false;

            unsigned int subscriberId = 0;

            //Act
            subscriberId = clientPeer->SubscribeToOnLocalPeerDisconnect(callback);
            serverPeer->Start();
            clientPeer->Start();

            while (isRunning)
            {
                timeClock.UpdateLocalTime();
                accumulator += timeClock.GetElapsedTimeSeconds();

                while (accumulator >= FIXED_FRAME_TARGET_DURATION)
                {
                    TickPeer(*serverPeer, FIXED_FRAME_TARGET_DURATION);
                    TickPeer(*clientPeer, FIXED_FRAME_TARGET_DURATION);

                    accumulator -= FIXED_FRAME_TARGET_DURATION;
                    disconnectServerTimeout -= FIXED_FRAME_TARGET_DURATION;
                    if (disconnectServerTimeout <= 0.f && !isAlreadyDisconnected)
                    {
                        serverPeer->Stop();
                        isAlreadyDisconnected = true;
                    }

                    testTimeLeft -= FIXED_FRAME_TARGET_DURATION;
                    if (testTimeLeft <= 0.f)
                    {
                        isRunning = false;
                    }
                }
            }

            //No need to call to clientPeer->Stop() since when it detects the server disconnects, the client also disconnects before executing callback

            clientPeer->UnsubscribeToOnPeerDisconnected(subscriberId);

            delete serverPeer;
            serverPeer = nullptr;
            delete clientPeer;
            clientPeer = nullptr;

            //Assert
            assert(numberOfTimesCalled == 1);
            assert(clientConnectionStateAfterStop == NetLib::PeerConnectionState::PCS_Disconnected);
            assert(disconnectionReason == NetLib::ConnectionFailedReasonType::CFR_PEER_SHUT_DOWN);

            //Tear down
            TearDown();

            return true;
        }

        bool static Test_ClientOnLocalPeerDisconnectDelegate_CheckItIsCalledOnlyOnceWhenServerDenyConnectionDueToItIsFull()
        {
            LogTestUtils::LogTestName("Test_ClientOnLocalPeerDisconnectDelegate_CheckItIsCalledOnlyOnceWhenServerDenyConnectionDueToItIsFull");

            //Set up
            SetUp();

            //Arrange
            const float clientServerInactivityTimeout = 5;
            const unsigned int serverMaxConnections = 1;
            const float testTimeout = 2;

            NetLib::Peer* serverPeer = new NetLib::Server(serverMaxConnections);
            NetLib::Peer* clientPeer1 = new NetLib::Client(clientServerInactivityTimeout);
            NetLib::Peer* clientPeer2 = new NetLib::Client(clientServerInactivityTimeout);

            bool isClient1Connected = false;
            bool isClient2Initialized = false;

            int numberOfTimesCalled = 0;
            bool isRunning = true;
            NetLib::PeerConnectionState client2ConnectionStateAfterStop = NetLib::PeerConnectionState::PCS_Connected;
            NetLib::ConnectionFailedReasonType disconnectionReason = NetLib::ConnectionFailedReasonType::CFR_UNKNOWN;

            auto callback = [&isRunning, &numberOfTimesCalled, &client2ConnectionStateAfterStop, &clientPeer2, &disconnectionReason](NetLib::ConnectionFailedReasonType reason)
            {
                isRunning = false;
                ++numberOfTimesCalled;
                client2ConnectionStateAfterStop = clientPeer2->GetConnectionState();
                disconnectionReason = reason;
            };

            auto callbackClient1Connected = [&isClient1Connected]()
            {
                isClient1Connected = true;
            };

            NetLib::TimeClock& timeClock = NetLib::TimeClock::GetInstance();
            double accumulator = 0.0;
            float testTimeLeft = testTimeout;
            float disconnectServerTimeout = 1.f;
            bool isAlreadyDisconnected = false;

            unsigned int subscriberId = 0;
            unsigned int client1ConnectedSubscriberId = 0;

            //Act
            subscriberId = clientPeer2->SubscribeToOnLocalPeerDisconnect(callback);
            client1ConnectedSubscriberId = clientPeer1->SubscribeToOnLocalPeerConnect(callbackClient1Connected);
            serverPeer->Start();
            clientPeer1->Start();

            while (isRunning)
            {
                timeClock.UpdateLocalTime();
                accumulator += timeClock.GetElapsedTimeSeconds();

                while (accumulator >= FIXED_FRAME_TARGET_DURATION)
                {
                    if (isClient1Connected && !isClient2Initialized)
                    {
                        clientPeer2->Start();
                        isClient2Initialized = true;
                    }

                    TickPeer(*serverPeer, FIXED_FRAME_TARGET_DURATION);
                    TickPeer(*clientPeer1, FIXED_FRAME_TARGET_DURATION);

                    if (isClient2Initialized)
                    {
                        TickPeer(*clientPeer2, FIXED_FRAME_TARGET_DURATION);
                    }

                    accumulator -= FIXED_FRAME_TARGET_DURATION;
                    disconnectServerTimeout -= FIXED_FRAME_TARGET_DURATION;

                    testTimeLeft -= FIXED_FRAME_TARGET_DURATION;
                    if (testTimeLeft <= 0.f)
                    {
                        isRunning = false;
                    }
                }
            }


            serverPeer->Stop();
            clientPeer1->Stop();
            clientPeer1->UnsubscribeToOnPeerConnected(client1ConnectedSubscriberId);

            //No need to call to clientPeer2->Stop() since when it detects the connection denied, the client automatically stops before executing callback
            clientPeer2->UnsubscribeToOnPeerDisconnected(subscriberId);

            delete serverPeer;
            serverPeer = nullptr;
            delete clientPeer1;
            clientPeer1 = nullptr;
            delete clientPeer2;
            clientPeer2 = nullptr;

            //Assert
            assert(numberOfTimesCalled == 1);
            assert(client2ConnectionStateAfterStop == NetLib::PeerConnectionState::PCS_Disconnected);
            assert(disconnectionReason == NetLib::ConnectionFailedReasonType::CFR_SERVER_FULL);

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
                    TickPeer(*serverPeer, FIXED_FRAME_TARGET_DURATION);
                    TickPeer(*clientPeer, FIXED_FRAME_TARGET_DURATION);

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
                    TickPeer(*serverPeer, FIXED_FRAME_TARGET_DURATION);
                    TickPeer(*clientPeer, FIXED_FRAME_TARGET_DURATION);

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
            float disconnectServerTimeout = 0.3f;
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
                    TickPeer(*serverPeer, FIXED_FRAME_TARGET_DURATION);
                    TickPeer(*clientPeer, FIXED_FRAME_TARGET_DURATION);

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

            //No need to call to clientPeer->Stop() since when it detects the server disconnects, the client also disconnects before executing callback
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
