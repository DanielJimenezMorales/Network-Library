#include "replication_messages_processor.h"

#include "numeric_types.h"
#include "logger.h"

#include "core/buffer.h"

#include "communication/message.h"

#include "replication/replication_action_type.h"
#include "replication/network_entity_communication_callbacks.h"
#include "replication/on_network_entity_create_config.h"

#include <cassert>

namespace NetLib
{
	ReplicationMessagesProcessor::ReplicationMessagesProcessor()
	    : _networkEntitiesStorage()
	    , _localPeerId( 0 )
	{
	}

	void ReplicationMessagesProcessor::Client_ProcessReceivedReplicationMessage(
	    const ReplicationMessage& replicationMessage )
	{
		ReplicationActionType type = static_cast< ReplicationActionType >( replicationMessage.replicationAction );
		switch ( type )
		{
			case ReplicationActionType::CREATE:
				ProcessReceivedCreateReplicationMessage( replicationMessage );
				break;
			case ReplicationActionType::UPDATE:
				ProcessReceivedUpdateReplicationMessage( replicationMessage );
				break;
			case ReplicationActionType::DESTROY:
				ProcessReceivedDestroyReplicationMessage( replicationMessage );
				break;
			default:
				LOG_WARNING( "Invalid replication action. Skipping it..." );
		}
	}

	void ReplicationMessagesProcessor::SetLocalClientId( uint32 id )
	{
		_localPeerId = id;
	}

	void ReplicationMessagesProcessor::ProcessReceivedCreateReplicationMessage(
	    const ReplicationMessage& replicationMessage )
	{
		uint32 networkEntityId = replicationMessage.networkEntityId;
		if ( _networkEntitiesStorage.HasNetworkEntityId( networkEntityId ) )
		{
			LOG_INFO( "Replication: Trying to create a network entity that is already created. Entity ID: %u. Ignoring "
			          "message...",
			          networkEntityId );
			return;
		}

		NetworkEntityData& new_entity_data = _networkEntitiesStorage.AddNetworkEntity(
		    replicationMessage.replicatedClassId, networkEntityId, replicationMessage.controlledByPeerId );

		// Create network entity through its custom factory
		Buffer buffer( replicationMessage.data, replicationMessage.dataSize );
		LOG_INFO( "DATA SIZE: %hu", replicationMessage.dataSize );
		const float32 posX = buffer.ReadFloat();
		const float32 posY = buffer.ReadFloat();

		OnNetworkEntityCreateConfig network_entity_create_config;
		network_entity_create_config.entityType = replicationMessage.replicatedClassId;
		network_entity_create_config.entityId = replicationMessage.networkEntityId;
		network_entity_create_config.controlledByPeerId = replicationMessage.controlledByPeerId;
		network_entity_create_config.positionX = posX;
		network_entity_create_config.positionY = posY;
		network_entity_create_config.communicationCallbacks = &new_entity_data.communicationCallbacks;
		// TODO Also evaluate if we need inGameId for something
		const int32 gameEntity = _onNetworkEntityCreate( network_entity_create_config );

		assert( gameEntity != -1 );

		new_entity_data.inGameId = static_cast< uint32 >( gameEntity );
	}

	void ReplicationMessagesProcessor::ProcessReceivedUpdateReplicationMessage(
	    const ReplicationMessage& replicationMessage )
	{
		uint32 networkEntityId = replicationMessage.networkEntityId;
		if ( !_networkEntitiesStorage.HasNetworkEntityId( networkEntityId ) )
		{
			LOG_INFO( "Replication: Trying to update a network entity that doesn't exist. Entity ID: %u. Creating a "
			          "new entity...",
			          networkEntityId );

			NetworkEntityData& new_entity_data = _networkEntitiesStorage.AddNetworkEntity(
			    replicationMessage.replicatedClassId, networkEntityId, replicationMessage.controlledByPeerId );

			// If not found create a new one and update it
			OnNetworkEntityCreateConfig network_entity_create_config;
			network_entity_create_config.entityType = replicationMessage.replicatedClassId;
			network_entity_create_config.entityId = replicationMessage.networkEntityId;
			network_entity_create_config.controlledByPeerId = replicationMessage.controlledByPeerId;
			network_entity_create_config.positionX = 0.f;
			network_entity_create_config.positionY = 0.f;
			network_entity_create_config.communicationCallbacks = &new_entity_data.communicationCallbacks;
			// TODO Also evaluate if we need inGameId for something
			const int32 gameEntity = _onNetworkEntityCreate( network_entity_create_config );

			assert( gameEntity != -1 );

			new_entity_data.inGameId = static_cast< uint32 >( gameEntity );
			return;
		}

		NetworkEntityData* entity_data = _networkEntitiesStorage.TryGetNetworkEntityFromId( networkEntityId );
		assert( entity_data != nullptr );

		// TODO Pass entity state to target entity
		Buffer buffer( replicationMessage.data, replicationMessage.dataSize );

		if ( entity_data->controlledByPeerId == _localPeerId )
		{
			entity_data->communicationCallbacks.OnUnserializeEntityStateForOwner.Execute( buffer );
		}
		else
		{
			entity_data->communicationCallbacks.OnUnserializeEntityStateForNonOwner.Execute( buffer );
		}
	}

	void ReplicationMessagesProcessor::ProcessReceivedDestroyReplicationMessage(
	    const ReplicationMessage& replicationMessage )
	{
		uint32 networkEntityId = replicationMessage.networkEntityId;
		RemoveNetworkEntity( networkEntityId );
	}

	void ReplicationMessagesProcessor::RemoveNetworkEntity( uint32 networkEntityId )
	{
		// Get game entity Id from network entity Id
		const NetworkEntityData* gameEntity = _networkEntitiesStorage.TryGetNetworkEntityFromId( networkEntityId );
		if ( gameEntity == nullptr )
		{
			LOG_INFO( "Replication: Trying to remove a network entity that doesn't exist. Network entity ID: %u. "
			          "Ignoring it...",
			          networkEntityId );
			return;
		}

		// Destroy object
		_onNetworkEntityDestroy( gameEntity->inGameId );
	}
} // namespace NetLib
