#pragma once
#include "replication/network_entity_storage.h"

namespace NetLib
{
	class ReplicationMessage;
	struct OnNetworkEntityCreateConfig;

	class ReplicationMessagesProcessor
	{
		public:
			ReplicationMessagesProcessor();

			void Client_ProcessReceivedReplicationMessage( const ReplicationMessage& replicationMessage );

			void SetLocalClientId( uint32 id );

			template < typename Functor >
			uint32 SubscribeToOnNetworkEntityCreate( Functor&& functor );

			template < typename Functor >
			uint32 SubscribeToOnNetworkEntityDestroy( Functor&& functor );

		private:
			void ProcessReceivedCreateReplicationMessage( const ReplicationMessage& replicationMessage );
			void ProcessReceivedUpdateReplicationMessage( const ReplicationMessage& replicationMessage );
			void ProcessReceivedDestroyReplicationMessage( const ReplicationMessage& replicationMessage );

			void RemoveNetworkEntity( uint32 networkEntityId );

			NetworkEntityStorage _networkEntitiesStorage;
			std::function< void( const OnNetworkEntityCreateConfig& ) > _onNetworkEntityCreate;
			std::function< void( uint32 ) > _onNetworkEntityDestroy;

			uint32 _localPeerId;
	};

	template < typename Functor >
	inline uint32 ReplicationMessagesProcessor::SubscribeToOnNetworkEntityCreate( Functor&& functor )
	{
		_onNetworkEntityCreate = std::move( functor );
		return 0;
	}

	template < typename Functor >
	inline uint32 ReplicationMessagesProcessor::SubscribeToOnNetworkEntityDestroy( Functor&& functor )
	{
		_onNetworkEntityDestroy = std::move( functor );
		return 0;
	}
} // namespace NetLib
