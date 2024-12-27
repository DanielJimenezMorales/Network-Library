#include "replication_manager.h"

#include <cassert>

#include "logger.h"

#include "communication/message_factory.h"

#include "replication/i_network_entity_factory.h"
#include "replication/replication_action_type.h"
#include "replication/network_entity_factory_registry.h"

namespace NetLib
{
	NetworkEntityData& ReplicationManager::SpawnNewNetworkEntity( uint32 replicated_class_id, uint32 network_entity_id,
	                                                              uint32 controlled_by_peer_id, float32 pos_x,
	                                                              float32 pos_y )
	{
		NetworkEntityData& new_entity_data =
		    _networkEntitiesStorage.AddNetworkEntity( replicated_class_id, network_entity_id, controlled_by_peer_id );

		// Create network entity through its custom factory
		const int32 gameEntity = _networkEntityFactoryRegistry->CreateNetworkEntity(
		    replicated_class_id, network_entity_id, controlled_by_peer_id, pos_x, pos_y,
		    new_entity_data.communicationCallbacks );
		assert( gameEntity != -1 );

		new_entity_data.inGameId = static_cast< uint32 >( gameEntity );
		return new_entity_data;
	}

	std::unique_ptr< ReplicationMessage > ReplicationManager::CreateCreateReplicationMessage( uint32 entityType,
	                                                                                          uint32 controlledByPeerId,
	                                                                                          uint32 networkEntityId,
	                                                                                          const Buffer& dataBuffer )
	{
		// Get message from message factory
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr< Message > message = messageFactory.LendMessage( MessageType::Replication );

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
	std::unique_ptr< ReplicationMessage > ReplicationManager::CreateUpdateReplicationMessage( uint32 entityType,
	                                                                                          uint32 networkEntityId,
	                                                                                          uint32 controlledByPeerId,
	                                                                                          const Buffer& buffer )
	{
		// Get message from message factory
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr< Message > message = messageFactory.LendMessage( MessageType::Replication );

		// Set reliability and order
		message->SetOrdered( true );
		message->SetReliability( false );

		// Set specific replication message data
		std::unique_ptr< ReplicationMessage > replicationMessage(
		    static_cast< ReplicationMessage* >( message.release() ) );
		replicationMessage->replicationAction = static_cast< uint8 >( ReplicationActionType::UPDATE );
		replicationMessage->networkEntityId = networkEntityId;
		replicationMessage->controlledByPeerId = controlledByPeerId;
		replicationMessage->dataSize = buffer.GetSize();
		replicationMessage->data = buffer.GetData();

		return std::move( replicationMessage );
	}

	std::unique_ptr< ReplicationMessage > ReplicationManager::CreateDestroyReplicationMessage( uint32 networkEntityId )
	{
		// Get message from message factory
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr< Message > message = messageFactory.LendMessage( MessageType::Replication );

		// Set reliability and order
		message->SetOrdered( true );
		message->SetReliability( true );

		// Set specific replication message data
		std::unique_ptr< ReplicationMessage > replicationMessage(
		    static_cast< ReplicationMessage* >( message.release() ) );
		replicationMessage->replicationAction = static_cast< uint8 >( ReplicationActionType::DESTROY );
		replicationMessage->networkEntityId = networkEntityId;

		return std::move( replicationMessage );
	}

	uint32 ReplicationManager::CreateNetworkEntity( uint32 entityType, uint32 controlledByPeerId, float32 posX,
	                                                float32 posY )
	{
		NetworkEntityData& new_entity_data =
		    SpawnNewNetworkEntity( entityType, _nextNetworkEntityId, controlledByPeerId, posX, posY );

		// Prepare a Create replication message for interested clients
		uint8* data = new uint8[ 8 ];
		Buffer buffer( data, 8 );
		buffer.WriteFloat( posX );
		buffer.WriteFloat( posY );
		std::unique_ptr< ReplicationMessage > createMessage =
		    CreateCreateReplicationMessage( entityType, controlledByPeerId, _nextNetworkEntityId, buffer );

		// Store it into queue before broadcasting it
		_pendingReplicationActionMessages.push_back( std::move( createMessage ) );

		CalculateNextNetworkEntityId();

		return new_entity_data.inGameId;
	}

	void ReplicationManager::RemoveNetworkEntity( uint32 networkEntityId )
	{
		// Get game entity Id from network entity Id
		NetworkEntityData gameEntity;
		bool foundSuccesfully = _networkEntitiesStorage.TryGetNetworkEntityFromId( networkEntityId, gameEntity );
		if ( !foundSuccesfully )
		{
			LOG_INFO( "Replication: Trying to remove a network entity that doesn't exist. Network entity ID: %u. "
			          "Ignoring it...",
			          networkEntityId );
			return;
		}

		// Destroy object through its custom factory
		_networkEntityFactoryRegistry->RemoveNetworkEntity( gameEntity.inGameId );

		std::unique_ptr< ReplicationMessage > destroyMessage = CreateDestroyReplicationMessage( networkEntityId );

		// Store it into queue before broadcasting it
		_pendingReplicationActionMessages.push_back( std::move( destroyMessage ) );
	}

	void ReplicationManager::Server_ReplicateWorldState(
	    uint32 remote_peer_id, std::vector< std::unique_ptr< ReplicationMessage > >& replication_messages )
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		auto cit = _pendingReplicationActionMessages.cbegin();
		for ( ; cit != _pendingReplicationActionMessages.cend(); ++cit )
		{
			const ReplicationMessage* source_replication_message = cit->get();

			std::unique_ptr< Message > message = messageFactory.LendMessage( MessageType::Replication );
			std::unique_ptr< ReplicationMessage > replicationMessage(
			    static_cast< ReplicationMessage* >( message.release() ) );

			const MessageHeader h = source_replication_message->GetHeader();
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

		for ( ; entity_it != itPastToEnd; ++entity_it )
		{
			NetworkEntityData& networkEntityData = entity_it->second;

			// TODO Remove this hardcoded size
			uint8* data = new uint8[ 64 ];
			Buffer buffer( data, 64 );
			if ( networkEntityData.controlledByPeerId == remote_peer_id )
			{
				networkEntityData.communicationCallbacks.OnSerializeEntityStateForOwner.Execute( buffer );
			}
			else
			{
				networkEntityData.communicationCallbacks.OnSerializeEntityStateForNonOwner.Execute( buffer );
			}

			std::unique_ptr< ReplicationMessage > message = CreateUpdateReplicationMessage(
			    networkEntityData.entityType, networkEntityData.id, networkEntityData.controlledByPeerId, buffer );
			replication_messages.push_back( std::move( message ) );
		}
	}

	void ReplicationManager::ClearSentReplicationMessages()
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();

		auto it = _pendingReplicationActionMessages.begin();
		for ( ; it != _pendingReplicationActionMessages.end(); ++it )
		{
			messageFactory.ReleaseMessage( std::move( *it ) );
		}
		_pendingReplicationActionMessages.clear();
	}

	void ReplicationManager::CalculateNextNetworkEntityId()
	{
		++_nextNetworkEntityId;

		if ( _nextNetworkEntityId == INVALID_NETWORK_ENTITY_ID )
		{
			++_nextNetworkEntityId;
		}
	}
} // namespace NetLib
