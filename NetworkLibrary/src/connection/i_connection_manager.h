#pragma once
#include "numeric_types.h"

#include "core/address.h"
#include "connection/pending_connection.h"

#include <unordered_map>

namespace NetLib
{
	class IConnectionPipeline;

	struct ConnectionConfiguration
	{
			uint32 maxPendingConnections;
			bool canStartConnections;
			IConnectionPipeline* connectionPipeline;
	};

	struct PendingConnectionData
	{
			PendingConnectionData( const Address& address, uint16 id, uint64 data_prefix )
			    : address( address )
			    , id( id )
			    , dataPrefix( data_prefix )
			{
			}

			Address address;
			uint16 id;
			uint64 dataPrefix;
	};

	class ConnectionManager
	{
		public:
			ConnectionManager( MessageFactory* message_factory );

			bool StartUp( ConnectionConfiguration& configuration );
			bool ShutDown();

			/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
			/	brief: Updates the connection manager and all its subsystems
			/
			/	param elapsed_time: The time since the last tick
			>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
			void Tick( float32 elapsed_time );

			/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
			/	brief: Checks if a pending connection to the specified address already exists
			/
			/	param address: The address to check
			/
			/	returns: true if exists, false otherwise
			>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
			bool DoesPendingConnectionExist( const Address& address ) const;

			/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
			/	brief: Adds a message received to a pending connection
			/
			/	param address: The pending connection's address
			/	param message: The received message
			/
			/	returns: true if added, false otherwise
			>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
			bool AddIncomingMessageToPendingConnection( const Address& address, std::unique_ptr< Message > message );

			/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
			/	brief: Creates a new pending connection based on an address
			/
			/	param address: The new pending connection's address
			/
			/	returns: true if created, false otherwise
			>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
			bool CreatePendingConnection( const Address& address );

			/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
			/	brief: Starts connecting to a specified address
			/
			/	notes: If a pending connection already exists with address, a new ConnectionMessage will be sent
			/
			/	param address: The address to start connection with
			/
			/	returns: true if connection started, false otherwise
			>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
			bool StartConnectingToAddress( const Address& address );

			void GetConnectedPendingConnectionsData(
			    std::vector< PendingConnectionData >& out_connected_pending_connections );

			void ClearConnectedPendingConnections();

		private:
			bool _isStartedUp;

			std::unordered_map< Address, PendingConnection, AddressHasher > _pendingConnections;
			IConnectionPipeline* _connectionPipeline;
			MessageFactory* _messageFactory;

			uint32 _maxPendingConnections;
			bool _canStartConnections;
	};
} // namespace NetLib
