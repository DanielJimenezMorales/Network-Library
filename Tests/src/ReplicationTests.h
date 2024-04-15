#pragma once
#include <cassert>
#include <chrono>
#include <thread>

#include "Server.h"
#include "Client.h"
#include "Initializer.h"
#include "LogTestUtils.h"
#include "INetworkEntity.h"
#include "NetworkEntityFactory.h"
#include "Delegate.h"

namespace Tests
{
    class TestNetworkEntity : public NetLib::INetworkEntity
    {
    public:
        TestNetworkEntity() : _networkEntityId(NetLib::INVALID_NETWORK_ENTITY_ID), _networkEntityType(TEST_ENTITY_TYPE), _numberOfTimesNetworkEntityCreateIsExecuted(0) {}

        uint32_t GetEntityId() const override { return _networkEntityId; }
        void SetEntityId(uint32_t entityId) override { _networkEntityId = entityId; }
        uint32_t GetEntityType() const override { return _networkEntityType; }

        unsigned int GetNumberOfTimesNetworkEntityCreateIsExecuted() const { return _numberOfTimesNetworkEntityCreateIsExecuted; }

        void NetworkEntityCreate() override
        {
            ++_numberOfTimesNetworkEntityCreateIsExecuted;
        }

        void NetworkEntityDestroy() override
        {
        }

        static const uint32_t TEST_ENTITY_TYPE = 1;

    private:
        uint32_t _networkEntityId;
        uint32_t _networkEntityType;

        unsigned int _numberOfTimesNetworkEntityCreateIsExecuted;
    };

    class TestNetworkEntityFactory : public NetLib::NetworkEntityFactory
    {
        NetLib::INetworkEntity& Create() override
        {
            TestNetworkEntity* newEntity = new TestNetworkEntity();
            return *newEntity;
        }

        void Destroy(NetLib::INetworkEntity& networkEntity) override
        {
            delete &networkEntity;
        }
    };

	class ReplicationTests
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
            //Set a small break between different tests in order to avoid receiving messages from the last test
            std::chrono::milliseconds duration(500);

            //Test local peer connection
            LogTestUtils::LogTestResult(Test_ServerNetworkEntityCreation());
            std::this_thread::sleep_for(duration);
            return true;
        }

        bool static Test_ServerNetworkEntityCreation()
        {
            LogTestUtils::LogTestName("Test_ServerNetworkEntityCreation");

            //Set up
            SetUp();

            //Arrange
            const float clientServerInactivityTimeout = 5;
            const unsigned int serverMaxConnections = 1;
            const float testTimeout = 2;

            NetLib::Server* serverPeer = new NetLib::Server(serverMaxConnections);

            int numberOfTimesCalled = 0;

            NetLib::INetworkEntity* networkEntity = nullptr;

            //Act
            serverPeer->Start();

            TestNetworkEntityFactory* serverTestNetworkEntityFactory = new TestNetworkEntityFactory();
            if (!serverPeer->RegisterNetworkEntityFactory(serverTestNetworkEntityFactory, TestNetworkEntity::TEST_ENTITY_TYPE))
            {

                Common::LOG_INFO("HHHHHH");
            }

            networkEntity = serverPeer->CreateNetworkEntity(TestNetworkEntity::TEST_ENTITY_TYPE);
            TestNetworkEntity* testNetworkEntity = static_cast<TestNetworkEntity*>(networkEntity);
            numberOfTimesCalled = testNetworkEntity->GetNumberOfTimesNetworkEntityCreateIsExecuted();

            serverPeer->Stop();

            delete serverPeer;
            serverPeer = nullptr;

            //Assert
            assert(numberOfTimesCalled == 1);

            //Tear down
            TearDown();

            return true;
        }
	};
}
