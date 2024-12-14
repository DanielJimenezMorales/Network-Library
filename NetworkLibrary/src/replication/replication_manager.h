#pragma once
#include "NumericTypes.h"
#include <unordered_map>
#include <queue>
#include <memory>
#include <functional>

#include "Message.h"
#include "Buffer.h"

#include "replication/network_variable_changes_handler.h"
#include "replication/network_entity_storage.h"

namespace NetLib
{
	class NetworkEntityFactoryRegistry;

	static constexpr uint32 INVALID_NETWORK_ENTITY_ID = 0;

	class ReplicationManager
	{
	public:
		ReplicationManager(NetworkEntityFactoryRegistry* networkEntityFactoryRegistry) : _nextNetworkEntityId(1), _networkEntityFactoryRegistry(networkEntityFactoryRegistry) {}

		uint32 CreateNetworkEntity(uint32 entityType, uint32 controlledByPeerId, float32 posX, float32 posY);
		void RemoveNetworkEntity(uint32 networkEntityId);

		void Server_ReplicateWorldState();
		void Client_ProcessReceivedReplicationMessage(const ReplicationMessage& replicationMessage);

		bool ArePendingReplicationMessages() const;
		const ReplicationMessage* GetPendingReplicationMessage();

		void ClearSentReplicationMessages();

	private:
		std::unique_ptr<ReplicationMessage> CreateCreateReplicationMessage(uint32 entityType, uint32 controlledByPeerId, uint32 networkEntityId, const Buffer& dataBuffer);
		std::unique_ptr<ReplicationMessage> CreateUpdateReplicationMessage(uint32 entityType, uint32 networkEntityId, uint32 controlledByPeerId, const Buffer& buffer);
		std::unique_ptr<ReplicationMessage> CreateDestroyReplicationMessage(uint32 networkEntityId);

		void ProcessReceivedCreateReplicationMessage(const ReplicationMessage& replicationMessage);
		void ProcessReceivedUpdateReplicationMessage(const ReplicationMessage& replicationMessage);
		void ProcessReceivedDestroyReplicationMessage(const ReplicationMessage& replicationMessage);

		void CalculateNextNetworkEntityId();

		NetworkEntityStorage _networkEntitiesStorage;

		std::queue<std::unique_ptr<ReplicationMessage>> _pendingReplicationActionMessages;
		std::queue<std::unique_ptr<ReplicationMessage>> _sentReplicationMessages;

		uint32 _nextNetworkEntityId;

		NetworkEntityFactoryRegistry* _networkEntityFactoryRegistry;
		NetworkVariableChangesHandler _networkVariableChangesHandler;
	};
}
