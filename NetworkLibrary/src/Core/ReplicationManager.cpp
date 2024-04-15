#include <sstream>
#include <cassert>

#include "ReplicationManager.h"
#include "Logger.h"
#include "MessageFactory.h"

namespace NetLib
{
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
		_pendingReplicationActionMessages.push(std::move(replicationMessage));
	}

	std::unique_ptr<ReplicationMessage> ReplicationManager::CreateUpdateReplicationMessage(uint32_t entityType, uint32_t networkEntityId)
	{
		//Get message from message factory
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr<Message> message = messageFactory.LendMessage(MessageType::Replication);

		//Set reliability and order
		message->SetOrdered(true);
		message->SetReliability(true);

		//Set specific replication message data
		std::unique_ptr<ReplicationMessage> replicationMessage(static_cast<ReplicationMessage*>(message.release()));
		replicationMessage->replicationAction = ReplicationActionType::RAT_UPDATE;
		replicationMessage->networkEntityId = networkEntityId;
		replicationMessage->replicatedClassId = entityType;

		return std::move(replicationMessage);
	}

	void ReplicationManager::CreateDestroyReplicationMessage(uint32_t networkEntityId)
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

		//Store it into queue before broadcasting it
		_pendingReplicationActionMessages.push(std::move(replicationMessage));
	}

	void ReplicationManager::ProcessReceivedCreateReplicationMessage(const ReplicationMessage& replicationMessage)
	{
		uint32_t networkEntityId = replicationMessage.networkEntityId;
		if (_networkEntitiesStorage.GetNetworkEntityFromId(networkEntityId) != nullptr)
		{
			std::stringstream ss;
			ss << "Replication: Trying to create a network entity that is already created. Entity ID: " << static_cast<int>(networkEntityId) << ".Ignoring message...";
			Common::LOG_INFO(ss.str());
			return;
		}

		//Create object through its custom factory
		INetworkEntity* networkEntity = _networkObjectsRegistry.CreateObjectOfType(replicationMessage.replicatedClassId);
		assert(networkEntity != nullptr);

		//Set Id
		networkEntity->SetEntityId(networkEntityId);

		//Add it to the network entities storage in order to avoid loosing it
		_networkEntitiesStorage.AddNetworkEntity(*networkEntity, false);
	}

	void ReplicationManager::ProcessReceivedUpdateReplicationMessage(const ReplicationMessage& replicationMessage)
	{
		uint32_t networkEntityId = replicationMessage.networkEntityId;
		if (_networkEntitiesStorage.GetNetworkEntityFromId(networkEntityId) == nullptr)
		{
			std::stringstream ss;
			ss << "Replication: Trying to update a network entity that doesn't exist. Entity ID: " << static_cast<int>(networkEntityId) << ".Creating a new entity...";
			Common::LOG_INFO(ss.str());

			ProcessReceivedCreateReplicationMessage(replicationMessage);
			return;
		}

		//TODO Pass entity state to target entity
	}

	void ReplicationManager::ProcessReceivedDestroyReplicationMessage(const ReplicationMessage& replicationMessage)
	{
		uint32_t networkEntityId = replicationMessage.networkEntityId;
		if (_networkEntitiesStorage.GetNetworkEntityFromId(networkEntityId) == nullptr)
		{
			std::stringstream ss;
			ss << "Replication: Trying to remove a network entity that doesn't exist. Entity ID: " << static_cast<int>(networkEntityId) << ".Ignoring it...";
			Common::LOG_INFO(ss.str());
			return;
		}

		RemoveNetworkEntity(networkEntityId);
	}

	bool ReplicationManager::RegisterNetworkEntityFactory(NetworkEntityFactory* entityFactory, uint32_t entityType)
	{
		return _networkObjectsRegistry.RegisterNetworkEntityFactory(entityFactory, entityType);
	}

	INetworkEntity* ReplicationManager::CreateNetworkEntity(uint32_t entityType)
	{
		//Create object through its custom factory
		INetworkEntity* networkEntity = _networkObjectsRegistry.CreateObjectOfType(entityType);
		assert(networkEntity != nullptr);

		//Add it to the network entities storage in order to avoid loosing it
		_networkEntitiesStorage.AddNetworkEntity(*networkEntity);

		networkEntity->NetworkEntityCreate();

		//Prepare a Create replication message for interested clients
		CreateCreateReplicationMessage(entityType, networkEntity->GetEntityId());

		return networkEntity;
	}

	void ReplicationManager::RemoveNetworkEntity(uint32_t entityId)
	{
		//Get Network Entity from Id
		INetworkEntity* networkEntityToRemove = _networkEntitiesStorage.GetNetworkEntityFromId(entityId);
		if (networkEntityToRemove == nullptr)
		{
			return;
		}

		networkEntityToRemove->NetworkEntityDestroy();

		//Destroy object through its custom factory
		_networkObjectsRegistry.DestroyObjectOfType(*networkEntityToRemove);

		CreateDestroyReplicationMessage(entityId);
	}

	void ReplicationManager::Server_ReplicateWorldState()
	{
		auto cit = _networkEntitiesStorage.GetNetworkEntities();
		auto citPastToEnd = _networkEntitiesStorage.GetPastToEndNetworkEntities();

		MessageFactory& messageFactory = MessageFactory::GetInstance();
		while (cit != citPastToEnd)
		{
			//TODO check that it has not been created with RAT_CREATE in the current tick
			std::unique_ptr<ReplicationMessage> message = CreateUpdateReplicationMessage(cit->second->GetEntityType(), cit->second->GetEntityId());
			_pendingReplicationActionMessages.push(std::move(message));
			++cit;
		}
	}

	void ReplicationManager::Client_ProcessReceivedReplicationMessage(const ReplicationMessage& replicationMessage)
	{
		ReplicationActionType type = static_cast<ReplicationActionType>(replicationMessage.replicationAction);
		switch (type)
		{
		case ReplicationActionType::RAT_CREATE:
			ProcessReceivedCreateReplicationMessage(replicationMessage);
			break;
		case ReplicationActionType::RAT_UPDATE:
			ProcessReceivedUpdateReplicationMessage(replicationMessage);
			break;
		case ReplicationActionType::RAT_DESTROY:
			ProcessReceivedDestroyReplicationMessage(replicationMessage);
			break;
		default:
			Common::LOG_WARNING("Invalid replication action. Skipping it...");
		}
	}

	bool ReplicationManager::ArePendingReplicationMessages() const
	{
		return !_pendingReplicationActionMessages.empty();
	}

	const ReplicationMessage* ReplicationManager::GetPendingReplicationMessage()
	{
		if (!ArePendingReplicationMessages())
		{
			return nullptr;
		}

		std::unique_ptr<ReplicationMessage> pendingReplicationMessage = std::move(_pendingReplicationActionMessages.front());
		_pendingReplicationActionMessages.pop();

		const ReplicationMessage* result = pendingReplicationMessage.get();

		_sentReplicationMessages.push(std::move(pendingReplicationMessage));

		return result;
	}

	void ReplicationManager::ClearSentReplicationMessages()
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		while (!_sentReplicationMessages.empty())
		{
			std::unique_ptr<ReplicationMessage> sentReplicationMessage = std::move(_sentReplicationMessages.front());
			_sentReplicationMessages.pop();

			messageFactory.ReleaseMessage(std::move(sentReplicationMessage));
		}
	}
}
