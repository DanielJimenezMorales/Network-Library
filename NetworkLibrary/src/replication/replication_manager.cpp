#include "replication_manager.h"

#include <cassert>

#include "Logger.h"

#include "communication/message_factory.h"

#include "replication/i_network_entity_factory.h"
#include "replication/replication_action_type.h"
#include "replication/network_entity_factory_registry.h"

namespace NetLib
{
	std::unique_ptr<ReplicationMessage> ReplicationManager::CreateCreateReplicationMessage(uint32 entityType, uint32 controlledByPeerId, uint32 networkEntityId, const Buffer& dataBuffer)
	{
		//Get message from message factory
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr<Message> message = messageFactory.LendMessage(MessageType::Replication);

		//Set reliability and order
		message->SetOrdered(true);
		message->SetReliability(true);

		//Set specific replication message data
		std::unique_ptr<ReplicationMessage> replicationMessage(static_cast<ReplicationMessage*>(message.release()));
		replicationMessage->replicationAction = static_cast<uint8>(ReplicationActionType::CREATE);
		replicationMessage->networkEntityId = networkEntityId;
		replicationMessage->controlledByPeerId = controlledByPeerId;
		replicationMessage->replicatedClassId = entityType;
		replicationMessage->data = dataBuffer.GetData();
		replicationMessage->dataSize = dataBuffer.GetSize();

		return std::move(replicationMessage);
	}

	//TODO Do we need the entity_type here too in case we need to create the entity from the update?
	std::unique_ptr<ReplicationMessage> ReplicationManager::CreateUpdateReplicationMessage(uint32 entityType, uint32 networkEntityId, uint32 controlledByPeerId, const Buffer& buffer)
	{
		//Get message from message factory
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr<Message> message = messageFactory.LendMessage(MessageType::Replication);

		//Set reliability and order
		message->SetOrdered(true);
		message->SetReliability(false);

		//Set specific replication message data
		std::unique_ptr<ReplicationMessage> replicationMessage(static_cast<ReplicationMessage*>(message.release()));
		replicationMessage->replicationAction = static_cast<uint8>(ReplicationActionType::UPDATE);
		replicationMessage->networkEntityId = networkEntityId;
		replicationMessage->controlledByPeerId = controlledByPeerId;
		replicationMessage->dataSize = buffer.GetSize();
		replicationMessage->data = buffer.GetData();

		return std::move(replicationMessage);
	}

	std::unique_ptr<ReplicationMessage> ReplicationManager::CreateDestroyReplicationMessage(uint32 networkEntityId)
	{
		//Get message from message factory
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr<Message> message = messageFactory.LendMessage(MessageType::Replication);

		//Set reliability and order
		message->SetOrdered(true);
		message->SetReliability(true);

		//Set specific replication message data
		std::unique_ptr<ReplicationMessage> replicationMessage(static_cast<ReplicationMessage*>(message.release()));
		replicationMessage->replicationAction = static_cast<uint8>(ReplicationActionType::DESTROY);
		replicationMessage->networkEntityId = networkEntityId;

		return std::move(replicationMessage);
	}

	void ReplicationManager::ProcessReceivedCreateReplicationMessage(const ReplicationMessage& replicationMessage)
	{
		uint32 networkEntityId = replicationMessage.networkEntityId;
		if (_networkEntitiesStorage.HasNetworkEntityId(networkEntityId))
		{
			LOG_INFO("Replication: Trying to create a network entity that is already created. Entity ID: %u. Ignoring message...", networkEntityId);
			return;
		}

		//Create network entity through its custom factory
		Buffer buffer(replicationMessage.data, replicationMessage.dataSize);
		LOG_INFO("DATA SIZE: %hu", replicationMessage.dataSize);
		float32 posX = buffer.ReadFloat();
		float32 posY = buffer.ReadFloat();
		int32 gameEntity = _networkEntityFactoryRegistry->CreateNetworkEntity(replicationMessage.replicatedClassId, networkEntityId, replicationMessage.controlledByPeerId, posX, posY, &_networkVariableChangesHandler);
		assert(gameEntity != -1);

		//Add it to the network entities storage in order to avoid loosing it
		_networkEntitiesStorage.AddNetworkEntity(replicationMessage.replicatedClassId, networkEntityId, replicationMessage.controlledByPeerId, gameEntity);
	}

	void ReplicationManager::ProcessReceivedUpdateReplicationMessage(const ReplicationMessage& replicationMessage)
	{
		uint32 networkEntityId = replicationMessage.networkEntityId;
		if (!_networkEntitiesStorage.HasNetworkEntityId(networkEntityId))
		{
			LOG_INFO("Replication: Trying to update a network entity that doesn't exist. Entity ID: %u. Creating a new entity...", networkEntityId);

			//If not found create a new one and update it
			int32 gameEntity = _networkEntityFactoryRegistry->CreateNetworkEntity(replicationMessage.replicatedClassId, networkEntityId, replicationMessage.controlledByPeerId, 0.f, 0.f, &_networkVariableChangesHandler);
			assert(gameEntity != -1);

			//Add it to the network entities storage in order to avoid loosing it
			_networkEntitiesStorage.AddNetworkEntity(replicationMessage.replicatedClassId, networkEntityId, replicationMessage.controlledByPeerId, gameEntity);
			return;
		}

		//TODO Pass entity state to target entity
		Buffer buffer(replicationMessage.data, replicationMessage.dataSize);
		_networkVariableChangesHandler.ProcessVariableChanges(buffer);
	}

	void ReplicationManager::ProcessReceivedDestroyReplicationMessage(const ReplicationMessage& replicationMessage)
	{
		uint32 networkEntityId = replicationMessage.networkEntityId;
		RemoveNetworkEntity(networkEntityId);
	}

	uint32 ReplicationManager::CreateNetworkEntity(uint32 entityType, uint32 controlledByPeerId, float32 posX, float32 posY)
	{
		//Create object through its custom factory
		int32 gameEntityId = _networkEntityFactoryRegistry->CreateNetworkEntity(entityType, _nextNetworkEntityId, controlledByPeerId, posX, posY, &_networkVariableChangesHandler);
		assert(gameEntityId != -1);

		//Add it to the network entities storage in order to avoid loosing it
		_networkEntitiesStorage.AddNetworkEntity(entityType, _nextNetworkEntityId, controlledByPeerId, gameEntityId);

		//Prepare a Create replication message for interested clients
		uint8* data = new uint8[8];
		Buffer buffer(data, 8);
		buffer.WriteFloat(posX);
		buffer.WriteFloat(posY);
		std::unique_ptr<ReplicationMessage> createMessage = CreateCreateReplicationMessage(entityType, controlledByPeerId, _nextNetworkEntityId, buffer);

		//Store it into queue before broadcasting it
		_pendingReplicationActionMessages.push(std::move(createMessage));

		CalculateNextNetworkEntityId();

		return static_cast<uint32>(gameEntityId);
	}

	void ReplicationManager::RemoveNetworkEntity(uint32 networkEntityId)
	{
		//Get game entity Id from network entity Id
		NetworkEntityData gameEntity;
		bool foundSuccesfully = _networkEntitiesStorage.TryGetNetworkEntityFromId(networkEntityId, gameEntity);
		if (!foundSuccesfully)
		{
			LOG_INFO("Replication: Trying to remove a network entity that doesn't exist. Network entity ID: %u. Ignoring it...", networkEntityId);
			return;
		}

		//Destroy object through its custom factory
		_networkEntityFactoryRegistry->RemoveNetworkEntity(gameEntity.inGameId);

		std::unique_ptr<ReplicationMessage> destroyMessage = CreateDestroyReplicationMessage(networkEntityId);

		//Store it into queue before broadcasting it
		_pendingReplicationActionMessages.push(std::move(destroyMessage));
	}

	void ReplicationManager::Server_ReplicateWorldState()
	{
		//Get all Network variable changes
		_networkVariableChangesHandler.CollectAllChanges();

		auto cit = _networkEntitiesStorage.GetNetworkEntities();
		auto citPastToEnd = _networkEntitiesStorage.GetPastToEndNetworkEntities();

		while (cit != citPastToEnd)
		{
			const NetworkEntityData networkEntityData = cit->second;

			//TODO check that it has not been created with RAT_CREATE in the current tick
			//Get network variable changes data
			const EntityNetworkVariableChanges* entityNetworkVariableChanges = _networkVariableChangesHandler.GetChangesFromEntity(networkEntityData.id);
			if (entityNetworkVariableChanges != nullptr)
			{
				uint32 dataSize = entityNetworkVariableChanges->Size() + sizeof(uint16);
				uint8* data = new uint8[dataSize];
				Buffer buffer(data, dataSize);

				//Write the number of network variables within this message
				uint16 numberOfChanges = entityNetworkVariableChanges->floatChanges.size();
				buffer.WriteShort(numberOfChanges);

				auto changesIt = entityNetworkVariableChanges->floatChanges.cbegin();
				for (; changesIt != entityNetworkVariableChanges->floatChanges.cend(); ++changesIt)
				{
					//TODO If we store multiple networkvariables of the same entity, we dont need to include each time the entityId. Create a number of variable changes
					//in order to only include entityId once
					buffer.WriteInteger(changesIt->networkVariableId);
					buffer.WriteInteger(changesIt->networkEntityId);
					buffer.WriteFloat(changesIt->value);
				}

				std::unique_ptr<ReplicationMessage> message = CreateUpdateReplicationMessage(networkEntityData.entityType, networkEntityData.id, networkEntityData.controlledByPeerId, buffer);
				_pendingReplicationActionMessages.push(std::move(message));
			}
			++cit;
		}

		_networkVariableChangesHandler.Clear();
	}

	void ReplicationManager::Client_ProcessReceivedReplicationMessage(const ReplicationMessage& replicationMessage)
	{
		ReplicationActionType type = static_cast<ReplicationActionType>(replicationMessage.replicationAction);
		switch (type)
		{
		case ReplicationActionType::CREATE:
			ProcessReceivedCreateReplicationMessage(replicationMessage);
			break;
		case ReplicationActionType::UPDATE:
			ProcessReceivedUpdateReplicationMessage(replicationMessage);
			break;
		case ReplicationActionType::DESTROY:
			ProcessReceivedDestroyReplicationMessage(replicationMessage);
			break;
		default:
			LOG_WARNING("Invalid replication action. Skipping it...");
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

	void ReplicationManager::CalculateNextNetworkEntityId()
	{
		++_nextNetworkEntityId;

		if (_nextNetworkEntityId == INVALID_NETWORK_ENTITY_ID)
		{
			++_nextNetworkEntityId;
		}
	}
}
