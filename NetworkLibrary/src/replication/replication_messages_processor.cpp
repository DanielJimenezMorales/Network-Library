#include "replication_messages_processor.h"

#include "Buffer.h"
#include "Logger.h"
#include "NumericTypes.h"

#include "communication/message.h"

#include "replication/replication_action_type.h"
#include "replication/network_entity_factory_registry.h"

#include <cassert>

namespace NetLib
{
	void ReplicationMessagesProcessor::Client_ProcessReceivedReplicationMessage(const ReplicationMessage& replicationMessage)
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

	void ReplicationMessagesProcessor::ProcessReceivedCreateReplicationMessage(const ReplicationMessage& replicationMessage)
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

	void ReplicationMessagesProcessor::ProcessReceivedUpdateReplicationMessage(const ReplicationMessage& replicationMessage)
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

	void ReplicationMessagesProcessor::ProcessReceivedDestroyReplicationMessage(const ReplicationMessage& replicationMessage)
	{
		uint32 networkEntityId = replicationMessage.networkEntityId;
		RemoveNetworkEntity(networkEntityId);
	}

	void ReplicationMessagesProcessor::RemoveNetworkEntity(uint32 networkEntityId)
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
	}
}
