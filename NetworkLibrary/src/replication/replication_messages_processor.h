#pragma once
#include "replication/network_variable_changes_handler.h"
#include "replication/network_entity_storage.h"

namespace NetLib
{
	class ReplicationMessage;
	class NetworkEntityFactoryRegistry;

	class ReplicationMessagesProcessor
	{
		public:
			ReplicationMessagesProcessor( NetworkEntityFactoryRegistry* networkEntityFactoryRegistry )
			    : _networkEntitiesStorage()
			    , _networkVariableChangesHandler()
			    , _networkEntityFactoryRegistry( networkEntityFactoryRegistry )
			{
			}

			void Client_ProcessReceivedReplicationMessage( const ReplicationMessage& replicationMessage );

		private:
			void ProcessReceivedCreateReplicationMessage( const ReplicationMessage& replicationMessage );
			void ProcessReceivedUpdateReplicationMessage( const ReplicationMessage& replicationMessage );
			void ProcessReceivedDestroyReplicationMessage( const ReplicationMessage& replicationMessage );

			void RemoveNetworkEntity( uint32 networkEntityId );

			NetworkEntityStorage _networkEntitiesStorage;
			NetworkEntityFactoryRegistry* _networkEntityFactoryRegistry;
			NetworkVariableChangesHandler _networkVariableChangesHandler;
	};
}
