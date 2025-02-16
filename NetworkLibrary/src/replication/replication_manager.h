#pragma once
#include "numeric_types.h"
#include "Delegate.h"

#include <memory>
#include <functional>

#include "core/buffer.h"

#include "communication/message.h"

#include "replication/network_entity_storage.h"

namespace NetLib
{
	struct OnNetworkEntityCreateConfig;

	static constexpr uint32 INVALID_NETWORK_ENTITY_ID = 0;

	class ReplicationManager
	{
		public:
			ReplicationManager();

			uint32 CreateNetworkEntity( uint32 entityType, uint32 controlledByPeerId, float32 posX, float32 posY );
			void RemoveNetworkEntity( uint32 networkEntityId );

			void Server_ReplicateWorldState(
			    uint32 remote_peer_id, std::vector< std::unique_ptr< ReplicationMessage > >& replication_messages );

			void ClearReplicationMessages();

			void RemoveNetworkEntitiesControllerByPeer( uint32 id );

			template < typename Functor >
			uint32 SubscribeToOnNetworkEntityCreate( Functor&& functor );

			template < typename Functor >
			uint32 SubscribeToOnNetworkEntityDestroy( Functor&& functor );

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

			std::function< uint32_t( const OnNetworkEntityCreateConfig& ) > _onNetworkEntityCreate;
			std::function< void( uint32 ) > _onNetworkEntityDestroy;
	};

	template < typename Functor >
	inline uint32 ReplicationManager::SubscribeToOnNetworkEntityCreate( Functor&& functor )
	{
		_onNetworkEntityCreate = std::move( functor );
		return 0;
	}
	template < typename Functor >
	inline uint32 ReplicationManager::SubscribeToOnNetworkEntityDestroy( Functor&& functor )
	{
		_onNetworkEntityDestroy = std::move( functor );
		return 0;
	}
} // namespace NetLib
