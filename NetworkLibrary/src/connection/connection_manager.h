#pragma once
#include "numeric_types.h"

#include "core/address.h"
#include "connection/pending_connection.h"
#include "connection/connection_failed_reason_type.h"

#include <unordered_map>

namespace NetLib
{
	class Socket;
	class NetworkPacket;

	namespace Connection
	{
		class IConnectionPipeline;

		struct ConnectionConfiguration
		{
				uint32 maxPendingConnections;
				bool canStartConnections;
				float32 connectionTimeoutSeconds;
				bool sendDenialOnTimeout;
				IConnectionPipeline* connectionPipeline;
		};

		struct PendingConnectionData
		{
				PendingConnectionData( const Address& address, bool started_locally, uint16 id, uint16 client_side_id,
				                       uint64 data_prefix )
				    : address( address )
				    , startedLocally( started_locally )
				    , id( id )
				    , clientSideId( client_side_id )
				    , dataPrefix( data_prefix )
				{
				}

				Address address;
				bool startedLocally;
				uint16 id;
				uint16 clientSideId;
				uint64 dataPrefix;
		};

		struct PendingConnectionFailedData
		{
				PendingConnectionFailedData( const Address& address, ConnectionFailedReasonType reason )
				    : address( address )
				    , reason( reason )
				{
				}

				Address address;
				ConnectionFailedReasonType reason;
		};

		class ConnectionManager
		{
			public:
				ConnectionManager();

				bool StartUp( ConnectionConfiguration& configuration, MessageFactory* message_factory );
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

				bool ProcessPacket( const Address& address, NetworkPacket& packet );

				/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
				/	brief: Creates a new pending connection based on an address
				/
				/	param address: The new pending connection's address
				/	param started_locally: Whether the connection was started locally or remotely
				/
				/	returns: true if created, false otherwise
				>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
				bool CreatePendingConnection( const Address& address, bool started_locally );

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

				void GetDeniedPendingConnectionsData(
				    std::vector< PendingConnectionFailedData >& out_denied_pending_connections );
				void ClearDeniedPendingConnections();

				void SendDataToPendingConnections( Socket& socket );

			private:
				void UpdateTimeout( PendingConnection& pending_connection, float32 elapsed_time );

				bool _isStartedUp;

				std::unordered_map< Address, PendingConnection, AddressHasher > _pendingConnections;
				IConnectionPipeline* _connectionPipeline;
				MessageFactory* _messageFactory;

				uint32 _maxPendingConnections;
				float32 _connectionTimeoutSeconds;
				bool _canStartConnections;
				bool _sendDenialOnTimeout;
		};
	} // namespace Connection
} // namespace NetLib
