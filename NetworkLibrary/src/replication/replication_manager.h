#pragma once
#include "numeric_types.h"

#include <memory>
#include <functional>

#include "core/buffer.h"

#include "communication/message.h"

#include "replication/network_entity_storage.h"

namespace NetLib
{
	class NetworkEntityFactoryRegistry;

	static constexpr uint32 INVALID_NETWORK_ENTITY_ID = 0;

	class ReplicationManager
	{
		public:
			ReplicationManager( NetworkEntityFactoryRegistry* networkEntityFactoryRegistry )
			    : _nextNetworkEntityId( 1 )
			    , _networkEntityFactoryRegistry( networkEntityFactoryRegistry )
			{
			}

			uint32 CreateNetworkEntity( uint32 entityType, uint32 controlledByPeerId, float32 posX, float32 posY );
			void RemoveNetworkEntity( uint32 networkEntityId );

			void Server_ReplicateWorldState(
			    uint32 remote_peer_id, std::vector< std::unique_ptr< ReplicationMessage > >& replication_messages );

			void ClearReplicationMessages();

			void RemoveNetworkEntitiesControllerByPeer( uint32 id );

		private:
			NetworkEntityData& SpawnNewNetworkEntity( uint32 replicated_class_id, uint32 network_entity_id,
			                                          uint32 controlled_by_peer_id, float32 pos_x, float32 pos_y );

			std::unique_ptr< ReplicationMessage > CreateCreateReplicationMessage( uint32 entityType,
			                                                                      uint32 controlledByPeerId,
			                                                                      uint32 networkEntityId,
			                                                                      const Buffer& dataBuffer );
			std::unique_ptr< ReplicationMessage > CreateUpdateReplicationMessage( uint32 entityType,
			                                                                      uint32 networkEntityId,
			                                                                      uint32 controlledByPeerId,
			                                                                      const Buffer& buffer );
			std::unique_ptr< ReplicationMessage > CreateDestroyReplicationMessage( uint32 networkEntityId );

			void CalculateNextNetworkEntityId();

			NetworkEntityStorage _networkEntitiesStorage;

			std::vector< std::unique_ptr< ReplicationMessage > > _createDestroyReplicationMessages;

			uint32 _nextNetworkEntityId;

			NetworkEntityFactoryRegistry* _networkEntityFactoryRegistry;
	};
} // namespace NetLib
