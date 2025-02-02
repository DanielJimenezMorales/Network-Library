#pragma once
#include "replication/network_variable_changes_handler.h"
#include "replication/network_entity_storage.h"

namespace NetLib
{
	class ReplicationMessage;
	class NetworkEntityFactoryRegistry;
	struct OnNetworkEntityCreateConfig;

	class ReplicationMessagesProcessor
	{
		public:
			ReplicationMessagesProcessor( NetworkEntityFactoryRegistry* networkEntityFactoryRegistry );

			void Client_ProcessReceivedReplicationMessage( const ReplicationMessage& replicationMessage );

			template < typename Functor >
			uint32 SubscribeToOnNetworkEntityCreate( Functor&& functor );

		private:
			void ProcessReceivedCreateReplicationMessage( const ReplicationMessage& replicationMessage );
			void ProcessReceivedUpdateReplicationMessage( const ReplicationMessage& replicationMessage );
			void ProcessReceivedDestroyReplicationMessage( const ReplicationMessage& replicationMessage );

			void RemoveNetworkEntity( uint32 networkEntityId );

			NetworkEntityStorage _networkEntitiesStorage;
			//NetworkEntityFactoryRegistry* _networkEntityFactoryRegistry;
			std::function< uint32_t( const OnNetworkEntityCreateConfig& ) > _onNetworkEntityCreate;
	};
	template < typename Functor >
	inline uint32 ReplicationMessagesProcessor::SubscribeToOnNetworkEntityCreate( Functor&& functor )
	{
		_onNetworkEntityCreate = std::move( functor );
		return 0;
	}
} // namespace NetLib
