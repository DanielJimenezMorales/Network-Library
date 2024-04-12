#include <sstream>
#include <cassert>

#include "ReplicationManager.h"
#include "Logger.h"
#include "MessageFactory.h"

namespace NetLib
{
	bool ReplicationManager::RegisterNetworkEntityFactory(NetworkEntityFactory& entityFactory, uint32_t entityType)
	{
		if (IsEntityFactoryRegistered(entityType))
		{
			std::stringstream ss;
			ss << "The Entity Factory of entity type " << static_cast<int>(entityType) << " is already registered. Skipping...";
			Common::LOG_WARNING(ss.str());
			return false;
		}

		_entityFactories[entityType] = &entityFactory;

		return true;
	}

	void ReplicationManager::CreateCreateReplicationMessage(uint32_t entityType, uint32_t networkEntityId)
	{
		//Get message from message factory
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr<Message> message = messageFactory.LendMessage(MessageType::Replication);

		//Set reliability and order
		message->SetOrdered(true);
		message->SetReliability(true);

		//Set specific replication message data
		std::unique_ptr<ReplicationMessage> replicationMessage(static_cast<ReplicationMessage*>(message.release()));
		replicationMessage->replicationAction = ReplicationActionType::RAT_CREATE;
		replicationMessage->networkEntityId = networkEntityId;
		replicationMessage->replicatedClassId = entityType;

		//Store it into queue before broadcasting it
		_pendingCreateReplicationActionMessages.push(std::move(replicationMessage));
	}

	void ReplicationManager::CreateDestroyReplicationMessage(uint32_t entityType, uint32_t networkEntityId)
	{
		//Get message from message factory
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr<Message> message = messageFactory.LendMessage(MessageType::Replication);

		//Set reliability and order
		message->SetOrdered(true);
		message->SetReliability(true);

		//Set specific replication message data
		std::unique_ptr<ReplicationMessage> replicationMessage(static_cast<ReplicationMessage*>(message.release()));
		replicationMessage->replicationAction = ReplicationActionType::RAT_DESTROY;
		replicationMessage->networkEntityId = networkEntityId;
		replicationMessage->replicatedClassId = entityType;

		//Store it into queue before broadcasting it
		_pendingDestroyReplicationActionMessages.push(std::move(replicationMessage));
	}

	NetworkEntityFactory* ReplicationManager::GetNetworkEntityFactory(uint32_t entityType)
	{
		NetworkEntityFactory* networkEntityFactory = nullptr;
		auto it = _entityFactories.find(entityType);
		if (it != _entityFactories.end())
		{
			networkEntityFactory = it->second;
		}

		return nullptr;
	}

	void ReplicationManager::CreateNetworkEntity(uint32_t entityType)
	{
		//Create object through its custom factory
		NetworkEntityFactory* networkEntityFactory = GetNetworkEntityFactory(entityType);
		assert(networkEntityFactory != nullptr);
		INetworkEntity& networkEntity = networkEntityFactory->Create();

		//Add it to the network entities storage in order to avoid loosing it
		_networkEntitiesStorage.AddNetworkEntity(networkEntity);

		//Prepare a Create replication message for interested clients
		CreateCreateReplicationMessage(entityType, networkEntity.GetEntityId());
	}

	void ReplicationManager::RemoveNetworkEntity(uint32_t entityId)
	{

	}

	void ReplicationManager::Tick()
	{
		auto cit = _networkEntitiesStorage.GetNetworkEntities();
		auto citPastToEnd = _networkEntitiesStorage.GetPastToEndNetworkEntities();

		while (cit != citPastToEnd)
		{
			
			++cit;
		}
	}

	bool ReplicationManager::IsEntityFactoryRegistered(uint32_t entityType) const
	{
		return _entityFactories.find(entityType) != _entityFactories.cend();
	}
}
