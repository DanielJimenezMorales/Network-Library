#pragma once
#include "numeric_types.h"

#include "core/address.h"
#include "connection/pending_connection.h"
#include "connection/connection_failed_reason_type.h"

#include <unordered_map>

/*
/	The Connection Manager is the main orchestrator of the Connection Component.
/
/	Responsabilities:
/		- Manage in-progress connections (timeouts, validations, etc)
/		- Handle incoming connection-related packets
/
/	Dependencies:
/		- Message Factory (For creating connection-related messages)
/		- Remote Peers Handler (For checking if there are empty remote peer slots for new connections)
/
*/
namespace NetLib
{
	class Socket;
	class NetworkPacket;
	class RemotePeersHandler;
	class MessageFactory;

	namespace Connection
	{
		class IConnectionPipeline;

		struct ConnectionConfiguration
		{
				// If true, this peer can send connection requests to other peers.
				bool canStartConnections;
				// Maximum time to wait for a connection to be established before timing out (in seconds).
				float32 connectionTimeoutSeconds;
				// If true, send a denial message when a connection times out.
				bool sendDenialOnTimeout;
				// The connection pipeline to use for processing connection states and messages.
				IConnectionPipeline* connectionPipeline;
		};

		struct SuccessConnectionData
		{
				SuccessConnectionData( const Address& address, bool started_locally, uint16 id, uint16 client_side_id,
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
				// The remote peer ID assigned by the server
				uint16 id;
				// The client-side ID assigned by the server (Use this variable when client opens a connection to the
				// server and the server assigns an ID to this client's local peer)
				uint16 clientSideId;
				uint64 dataPrefix;
		};

		struct FailedConnectionData
		{
				FailedConnectionData( const Address& address, ConnectionFailedReasonType reason )
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

				/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
				/	brief: Starts up the connection manager.
				/
				/	notes: Call this method before calling any other from the connection manager.
				/
				/	param configuration: The connection configuration to use
				/	param message_factory: The Message Factory dependency
				/	param remote_peers_handler: The Remote Peers Handler dependency
				/
				/	returns: true if started up successfully, false otherwise
				>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
				bool StartUp( ConnectionConfiguration& configuration, MessageFactory* message_factory,
				              const RemotePeersHandler* remote_peers_handler );

				/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
				/	brief: Shuts down the connection manager.
				/
				/	returns: true if shut down successfully, false otherwise
				>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
				bool ShutDown();

				/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
				/	brief: Process an incoming network packet that doesn't belong to any connected remote peer.
				/
				/	param address: The source address of the network packet
				/	param packet: The network packet to process
				/
				/	returns: true if processed successfully, false otherwise
				>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
				bool ProcessPacket( const Address& address, NetworkPacket& packet );

				/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
				/	brief: Updates the connection manager and all its subsystems
				/
				/	param elapsed_time: The time since the last tick
				>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
				void Tick( float32 elapsed_time );

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

				/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
				/	brief: Sends the pending connection-related messages, if any, to the remote connections.
				/
				/	param socket: The socket used to transmit the data through
				>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
				void SendData( Socket& socket );

				/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
				/	brief: Gets all the success connections that hasn't been removed yet.
				/
				/	notes: Do not forget to call RemoveSuccessConnections after processing the data.
				/
				/	param out_success_connections: An output array with the data of the success connections.
				>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
				void GetSuccessConnectionsData( std::vector< SuccessConnectionData >& out_success_connections );

				/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
				/	brief: Remove all the success connections to get some available slots for the new ones.
				/
				/	notes: Call this method after processing the success connections data, or you will lost them.
				>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
				void RemoveSuccessConnections();

				/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
				/	brief: Gets all the failed connections that hasn't been removed yet.
				/
				/	notes: Do not forget to call RemoveFailedConnections after processing the data.
				/
				/	param out_success_connections: An output array with the data of the failed connections.
				>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
				void GetFailedConnectionsData( std::vector< FailedConnectionData >& out_failed_connections );

				/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
				/	brief: Remove all the failed connections to get some available slots for the new ones.
				/
				/	notes: Call this method after processing the failed connections data, or you will lost them.
				>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
				void RemoveFailedConnections();

			private:
				/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
				/	brief: Checks if a pending connection to the specified address already exists
				/
				/	param address: The address to check
				/
				/	returns: true if exists, false otherwise
				>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
				bool DoesPendingConnectionExist( const Address& address ) const;

				/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
				/	brief: Creates a new pending connection based on an address
				/
				/	param address: The new pending connection's address
				/	param started_locally: Whether the connection was started in the local or remote peer
				/
				/	returns: true if created, false otherwise
				>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
				bool CreatePendingConnection( const Address& address, bool started_locally );

				/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
				/	brief: Check if it's possible to host another new connection
				/
				/	returns: true if possible, false otherwise
				>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
				bool AreSlotsAvailableForNewPendingConnection() const;

				/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
				/	brief: Updates the pending connection's elapsed times.
				/
				/	notes: If any exceeds the maximum elapsed time then force a connection failure.
				/
				/	param pending_connection: The new pending connection's to update
				/	param elapsed_time: The elapsed time since the last call
				>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
				void UpdateTimeout( PendingConnection& pending_connection, float32 elapsed_time );

				MessageFactory* _messageFactory;
				const RemotePeersHandler* _remotePeersHandler;

				bool _isStartedUp;
				std::unordered_map< Address, PendingConnection, AddressHasher > _pendingConnections;
				IConnectionPipeline* _connectionPipeline;
				float32 _connectionTimeoutSeconds;
				bool _canStartConnections;
				bool _sendDenialOnTimeout;
		};
	} // namespace Connection
} // namespace NetLib
