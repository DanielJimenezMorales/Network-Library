#pragma once
#include <cstdint>
#include <unordered_map>
#include <queue>
#include <memory>

#include "NetworkEntityStorage.h"
#include "ReplicationObjectRegistry.h"
#include "Message.h"

namespace NetLib
{
	enum ReplicationActionType : uint8_t
	{
		RAT_CREATE = 0,
		RAT_UPDATE = 1,
		RAT_DESTROY = 2
	};

	class ReplicationManager
	{
	public:
		bool RegisterNetworkEntityFactory(NetworkEntityFactory* entityFactory, uint32_t entityType);
		INetworkEntity* CreateNetworkEntity(uint32_t entityType);
		void RemoveNetworkEntity(uint32_t entityId);

		void Server_ReplicateWorldState();
		void Client_ProcessReceivedReplicationMessage(const ReplicationMessage& replicationMessage);

		bool ArePendingReplicationMessages() const;
		const ReplicationMessage* GetPendingReplicationMessage();

		void ClearSentReplicationMessages();

	private:
		void CreateCreateReplicationMessage(uint32_t entityType, uint32_t networkEntityId);
		std::unique_ptr<ReplicationMessage> CreateUpdateReplicationMessage(uint32_t entityType, uint32_t networkEntityId);
		void CreateDestroyReplicationMessage(uint32_t networkEntityId);

		void ProcessReceivedCreateReplicationMessage(const ReplicationMessage& replicationMessage);
		void ProcessReceivedUpdateReplicationMessage(const ReplicationMessage& replicationMessage);
		void ProcessReceivedDestroyReplicationMessage(const ReplicationMessage& replicationMessage);

		NetworkEntityStorage _networkEntitiesStorage;
		ReplicationObjectRegistry _networkObjectsRegistry;

		std::queue<std::unique_ptr<ReplicationMessage>> _pendingReplicationActionMessages;
		std::queue<std::unique_ptr<ReplicationMessage>> _sentReplicationMessages;
	};
}
