#pragma once
#include <cstdint>
#include <unordered_map>
#include <queue>
#include <memory>

#include "NetworkEntityFactory.h"
#include "NetworkEntityStorage.h"
#include "Message.h"

namespace NetLib
{
	enum ReplicationActionType : uint8_t
	{
		RAT_CREATE = 0,
		RAT_UPDATE = 1,
		RAT_DESTROY = 2
	};

	//SERVER ONLY
	class ReplicationManager
	{
	public:
		bool RegisterNetworkEntityFactory(NetworkEntityFactory& entityFactory, uint32_t entityType);
		void CreateNetworkEntity(uint32_t entityType);
		void RemoveNetworkEntity(uint32_t entityId);

		void Tick();

	private:
		void CreateCreateReplicationMessage(uint32_t entityType, uint32_t networkEntityId);
		void CreateDestroyReplicationMessage(uint32_t entityType, uint32_t networkEntityId);
		NetworkEntityFactory* GetNetworkEntityFactory(uint32_t entityType);
		bool IsEntityFactoryRegistered(uint32_t entityType) const;

		std::unordered_map<uint32_t, NetworkEntityFactory*> _entityFactories;
		NetworkEntityStorage _networkEntitiesStorage;

		std::queue<std::unique_ptr<Message>> _pendingCreateReplicationActionMessages;
		std::queue<std::unique_ptr<Message>> _pendingUpdateReplicationActionMessages;
		std::queue<std::unique_ptr<Message>> _pendingDestroyReplicationActionMessages;
	};
}
