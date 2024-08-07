#pragma once
#include <cstdint>
#include <unordered_map>
#include <queue>
#include <memory>
#include <functional>

#include "NetworkEntityStorage.h"
#include "Message.h"
#include "Buffer.h"
#include "NetworkVariableChangesHandler.h"

namespace NetLib
{
	class INetworkEntityFactory;

	static constexpr uint32_t INVALID_NETWORK_ENTITY_ID = 0;

	//TODO Convert this to enum class
	enum ReplicationActionType : uint8_t
	{
		RAT_CREATE = 0,
		RAT_UPDATE = 1,
		RAT_DESTROY = 2
	};

	class ReplicationManager
	{
	public:
		ReplicationManager() : _nextNetworkEntityId(1), _networkEntityFactory(nullptr) {}

		void RegisterNetworkEntityFactory(INetworkEntityFactory* entityFactory);
		uint32_t CreateNetworkEntity(uint32_t entityType, uint32_t controlledByPeerId, float posX, float posY);
		void RemoveNetworkEntity(uint32_t networkEntityId);

		void Server_ReplicateWorldState();
		void Client_ProcessReceivedReplicationMessage(const ReplicationMessage& replicationMessage);

		bool ArePendingReplicationMessages() const;
		const ReplicationMessage* GetPendingReplicationMessage();

		void ClearSentReplicationMessages();

	private:
		std::unique_ptr<ReplicationMessage> CreateCreateReplicationMessage(uint32_t entityType, uint32_t controlledByPeerId, uint32_t networkEntityId, const Buffer& dataBuffer);
		std::unique_ptr<ReplicationMessage> CreateUpdateReplicationMessage(uint32_t entityType, uint32_t networkEntityId, uint32_t controlledByPeerId, const Buffer& buffer);
		std::unique_ptr<ReplicationMessage> CreateDestroyReplicationMessage(uint32_t networkEntityId);

		void ProcessReceivedCreateReplicationMessage(const ReplicationMessage& replicationMessage);
		void ProcessReceivedUpdateReplicationMessage(const ReplicationMessage& replicationMessage);
		void ProcessReceivedDestroyReplicationMessage(const ReplicationMessage& replicationMessage);

		void CalculateNextNetworkEntityId();

		NetworkEntityStorage _networkEntitiesStorage;

		std::queue<std::unique_ptr<ReplicationMessage>> _pendingReplicationActionMessages;
		std::queue<std::unique_ptr<ReplicationMessage>> _sentReplicationMessages;

		uint32_t _nextNetworkEntityId;

		//TODO Support multiple network entity factories depending on the entity_type
		INetworkEntityFactory* _networkEntityFactory;
		NetworkVariableChangesHandler _networkVariableChangesHandler;
	};
}
