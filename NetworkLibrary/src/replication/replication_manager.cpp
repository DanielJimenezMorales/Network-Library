#include "replication_manager.h"

#include <cassert>

#include "logger.h"
#include "asserts.h"

#include "communication/message_factory.h"

#include "replication/replication_action_type.h"
#include "replication/on_network_entity_create_config.h"

namespace NetLib
{
	ReplicationManager::ReplicationManager()
	    : _nextNetworkEntityId( 1 )
	{
	}

	void ReplicationManager::SpawnNewNetworkEntity( uint32 replicated_class_id, uint32 network_entity_id,
	                                                uint32 controlled_by_peer_id, float32 pos_x, float32 pos_y )
	{
		// Create network entity associated data
		NetworkEntityData* new_entity_data =
		    _networkEntitiesStorage.AddNetworkEntity( replicated_class_id, network_entity_id, controlled_by_peer_id );

		ASSERT( new_entity_data != nullptr,
		        "Failed to create new network entity data for entity type %u with network entity id %u",
		        replicated_class_id, network_entity_id );

		// Spawn network entity in world
		OnNetworkEntityCreateConfig network_entity_create_config;
		network_entity_create_config.entityType = replicated_class_id;
		network_entity_create_config.entityId = network_entity_id;
		network_entity_create_config.controlledByPeerId = controlled_by_peer_id;
		network_entity_create_config.positionX = pos_x;
		network_entity_create_config.positionY = pos_y;
		network_entity_create_config.communicationCallbacks = &new_entity_data->communicationCallbacks;
		// TODO Also evaluate if we need inGameId for something
		_onNetworkEntityCreate( network_entity_create_config );
	}

	std::unique_ptr< ReplicationMessage > ReplicationManager::CreateCreateReplicationMessage(
	    MessageFactory& message_factory, uint32 entityType, uint32 controlledByPeerId, uint32 networkEntityId,
	    const Buffer& dataBuffer )
	{
		// Get message from message factory
		std::unique_ptr< Message > message = message_factory.LendMessage( MessageType::Replication );

		// Set reliability and order
		message->SetOrdered( true );
		message->SetReliability( true );

		// Set specific replication message data
		std::unique_ptr< ReplicationMessage > replicationMessage(
		    static_cast< ReplicationMessage* >( message.release() ) );
		replicationMessage->replicationAction = static_cast< uint8 >( ReplicationActionType::CREATE );
		replicationMessage->networkEntityId = networkEntityId;
		replicationMessage->controlledByPeerId = controlledByPeerId;
		replicationMessage->replicatedClassId = entityType;
		replicationMessage->data = dataBuffer.GetData();
		replicationMessage->dataSize = dataBuffer.GetSize();

		return std::move( replicationMessage );
	}

	// TODO Do we need the entity_type here too in case we need to create the entity from the update?
	std::unique_ptr< ReplicationMessage > ReplicationManager::CreateUpdateReplicationMessage(
	    MessageFactory& message_factory, uint32 entityType, uint32 networkEntityId, uint32 controlledByPeerId,
	    const Buffer& buffer )
	{
		// Get message from message factory
		std::unique_ptr< Message > message = message_factory.LendMessage( MessageType::Replication );

		// Set reliability and order
		message->SetOrdered( true );
		message->SetReliability( false );

		// Set specific replication message data
		std::unique_ptr< ReplicationMessage > replicationMessage(
		    static_cast< ReplicationMessage* >( message.release() ) );
		replicationMessage->replicationAction = static_cast< uint8 >( ReplicationActionType::UPDATE );
		replicationMessage->networkEntityId = networkEntityId;
		replicationMessage->controlledByPeerId = controlledByPeerId;
		replicationMessage->replicatedClassId = entityType;
		// TODO Use some write stream here instead of manual buffer
		replicationMessage->dataSize = buffer.GetAccessIndex();
		replicationMessage->data = new uint8[ replicationMessage->dataSize ];
		buffer.CopyUsedData( replicationMessage->data, replicationMessage->dataSize );

		return std::move( replicationMessage );
	}

	std::unique_ptr< ReplicationMessage > ReplicationManager::CreateDestroyReplicationMessage(
	    MessageFactory& message_factory, uint32 networkEntityId )
	{
		// Get message from message factory
		std::unique_ptr< Message > message = message_factory.LendMessage( MessageType::Replication );

		// Set reliability and order
		message->SetOrdered( true );
		message->SetReliability( true );

		// Set specific replication message data
		std::unique_ptr< ReplicationMessage > replicationMessage(
		    static_cast< ReplicationMessage* >( message.release() ) );
		replicationMessage->replicationAction = static_cast< uint8 >( ReplicationActionType::DESTROY );
		replicationMessage->networkEntityId = networkEntityId;
		replicationMessage->dataSize = 0;

		return std::move( replicationMessage );
	}

	void ReplicationManager::CreateNetworkEntity( MessageFactory& message_factory, uint32 entityType,
	                                              uint32 controlledByPeerId, float32 posX, float32 posY )
	{
		SpawnNewNetworkEntity( entityType, _nextNetworkEntityId, controlledByPeerId, posX, posY );

		// Prepare a Create replication message for interested clients
		uint8* data = new uint8[ 8 ];
		Buffer buffer( data, 8 );
		buffer.WriteFloat( posX );
		buffer.WriteFloat( posY );
		std::unique_ptr< ReplicationMessage > createMessage = CreateCreateReplicationMessage(
		    message_factory, entityType, controlledByPeerId, _nextNetworkEntityId, buffer );

		// Store it into queue before broadcasting it
		_createDestroyReplicationMessages.push_back( std::move( createMessage ) );

		CalculateNextNetworkEntityId();
	}

	void ReplicationManager::RemoveNetworkEntity( MessageFactory& message_factory, uint32 networkEntityId )
	{
		// Get game entity Id from network entity Id
		const NetworkEntityData* networkEntity = _networkEntitiesStorage.TryGetNetworkEntityFromId( networkEntityId );
		if ( networkEntity == nullptr )
		{
			LOG_INFO( "Replication: Trying to remove a network entity that doesn't exist. Network entity ID: %u. "
			          "Ignoring it...",
			          networkEntityId );
		}
		else
		{
			// Destroy object through its custom factory
			_onNetworkEntityDestroy( networkEntity->id );

			// Remove network enttiy data
			_networkEntitiesStorage.RemoveNetworkEntity( networkEntityId );

			// Create destroy entity message for remote peers
			std::unique_ptr< ReplicationMessage > destroyMessage =
			    CreateDestroyReplicationMessage( message_factory, networkEntityId );

			// Store it into queue before broadcasting it
			_createDestroyReplicationMessages.push_back( std::move( destroyMessage ) );
		}
	}

	void ReplicationManager::Server_ReplicateWorldState(
	    MessageFactory& message_factory, uint32 remote_peer_id,
	    std::vector< std::unique_ptr< ReplicationMessage > >& replication_messages )
	{
		auto cit = _createDestroyReplicationMessages.cbegin();
		for ( ; cit != _createDestroyReplicationMessages.cend(); ++cit )
		{
			const ReplicationMessage* source_replication_message = cit->get();

			std::unique_ptr< Message > message = message_factory.LendMessage( MessageType::Replication );
			std::unique_ptr< ReplicationMessage > replicationMessage(
			    static_cast< ReplicationMessage* >( message.release() ) );

			// TODO Create an operator= or something like that to avoid this spaguetti code
			replicationMessage->SetOrdered( source_replication_message->GetHeader().isOrdered );
			replicationMessage->SetReliability( source_replication_message->GetHeader().isReliable );
			replicationMessage->replicationAction = source_replication_message->replicationAction;
			replicationMessage->networkEntityId = source_replication_message->networkEntityId;
			replicationMessage->controlledByPeerId = source_replication_message->controlledByPeerId;
			replicationMessage->replicatedClassId = source_replication_message->replicatedClassId;
			replicationMessage->dataSize = source_replication_message->dataSize;
			if ( replicationMessage->dataSize > 0 )
			{
				// TODO Figure out if I can improve this. So far, for large snapshot updates data this can
				// become heavy and slow. Can I avoid the copy somehow?
				uint8* data = new uint8[ replicationMessage->dataSize ];
				std::memcpy( data, source_replication_message->data, replicationMessage->dataSize );
				replicationMessage->data = data;
			}

			replication_messages.push_back( std::move( replicationMessage ) );
		}

		auto entity_it = _networkEntitiesStorage.GetNetworkEntities();
		auto itPastToEnd = _networkEntitiesStorage.GetPastToEndNetworkEntities();

		// TODO Remove this hardcoded size
		const uint32 serialization_buffer_size = 128;
		uint8* data = new uint8[ serialization_buffer_size ];
		Buffer buffer( data, serialization_buffer_size );
		for ( ; entity_it != itPastToEnd; ++entity_it )
		{
			NetworkEntityData& networkEntityData = entity_it->second;

			if ( networkEntityData.controlledByPeerId == remote_peer_id )
			{
				networkEntityData.communicationCallbacks.OnSerializeEntityStateForOwner.Execute( buffer );
			}
			else
			{
				// TODO Here is an error
				networkEntityData.communicationCallbacks.OnSerializeEntityStateForNonOwner.Execute( buffer );
			}

			std::unique_ptr< ReplicationMessage > message = CreateUpdateReplicationMessage(message_factory,
			    networkEntityData.entityType, networkEntityData.id, networkEntityData.controlledByPeerId, buffer );
			replication_messages.push_back( std::move( message ) );

			buffer.Clear();
		}

		delete[] data;
	}

	void ReplicationManager::ClearReplicationMessages( MessageFactory& message_factory )
	{
		auto it = _createDestroyReplicationMessages.begin();
		for ( ; it != _createDestroyReplicationMessages.end(); ++it )
		{
			message_factory.ReleaseMessage( std::move( *it ) );
		}
		_createDestroyReplicationMessages.clear();
	}

	void ReplicationManager::RemoveNetworkEntitiesControllerByPeer(MessageFactory& message_factory,uint32 id )
	{
		std::vector< uint32 > network_entity_ids_to_remove;
		const std::unordered_map< uint32, NetworkEntityData >& network_entities =
		    _networkEntitiesStorage.GetNetworkEntitiess();

		auto cit = network_entities.cbegin();
		for ( ; cit != network_entities.cend(); ++cit )
		{
			const NetworkEntityData& network_entity_data = cit->second;
			if ( network_entity_data.controlledByPeerId == id )
			{
				network_entity_ids_to_remove.push_back( network_entity_data.id );
			}
		}

		auto ids_to_remove_cit = network_entity_ids_to_remove.cbegin();
		for ( ; ids_to_remove_cit != network_entity_ids_to_remove.cend(); ++ids_to_remove_cit )
		{
			RemoveNetworkEntity(message_factory ,*ids_to_remove_cit );
		}
	}

	void ReplicationManager::CalculateNextNetworkEntityId()
	{
		++_nextNetworkEntityId;

		if ( _nextNetworkEntityId == INVALID_NETWORK_ENTITY_ID )
		{
			LOG_WARNING( "Replication: The next network entity id has overflowed and it is being reset to 0 again." );
			++_nextNetworkEntityId;
		}
	}
} // namespace NetLib
