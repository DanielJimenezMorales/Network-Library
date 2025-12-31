#include "connection_manager.h"

#include "connection/i_connection_pipeline.h"
#include "communication/network_packet.h"
#include "communication/message.h"
#include "core/remote_peers_handler.h"

#include "logger.h"
#include "asserts.h"

namespace NetLib
{
	namespace Connection
	{
		ConnectionManager::ConnectionManager()
		    : _isStartedUp( false )
		    , _messageFactory( nullptr )
		    , _remotePeersHandler( nullptr )
		    , _pendingConnections()
		    , _connectionPipeline( nullptr )
		    , _connectionTimeoutSeconds( 0.f )
		    , _canStartConnections( false )
		    , _sendDenialOnTimeout( false )
		{
		}

		bool ConnectionManager::StartUp( ConnectionConfiguration& configuration, MessageFactory* message_factory,
		                                 const RemotePeersHandler* remote_peers_handler )
		{
			if ( _isStartedUp )
			{
				LOG_ERROR( "[ConnectionManager.%s] ConnectionManager is already started up, ignoring call",
				           THIS_FUNCTION_NAME );
				return false;
			}

			ASSERT( message_factory != nullptr, "Message factory can't be null" );
			ASSERT( remote_peers_handler != nullptr, "Remote peers handler can't be null" );

			_messageFactory = message_factory;
			_remotePeersHandler = remote_peers_handler;

			_pendingConnections.reserve( _remotePeersHandler->GetMaxConnections() );

			if ( configuration.connectionPipeline != nullptr )
			{
				_connectionPipeline = configuration.connectionPipeline;
				_canStartConnections = configuration.canStartConnections;
				_connectionTimeoutSeconds = configuration.connectionTimeoutSeconds;
				_sendDenialOnTimeout = configuration.sendDenialOnTimeout;

				_isStartedUp = true;
			}
			else
			{
				LOG_ERROR( "[ConnectionManager.%s] ConnectionManager cannot start up. Configuration has invalid fields",
				           THIS_FUNCTION_NAME );
			}

			return _isStartedUp;
		}

		bool ConnectionManager::ShutDown()
		{
			if ( !_isStartedUp )
			{
				LOG_ERROR( "[ConnectionManager.%s] ConnectionManager is not started up, ignoring call",
				           THIS_FUNCTION_NAME );
				return false;
			}

			// Shut down all pending connections
			for ( auto& it = _pendingConnections.begin(); it != _pendingConnections.end(); ++it )
			{
				if ( !it->second.ShutDown() )
				{
					LOG_ERROR( "[ConnectionManager.%s] Failed to shut down pending connection.", THIS_FUNCTION_NAME );
				}
			}
			_pendingConnections.clear();

			_messageFactory = nullptr;
			_remotePeersHandler = nullptr;

			// Shut down connection pipeline
			if ( _connectionPipeline != nullptr )
			{
				delete _connectionPipeline;
				_connectionPipeline = nullptr;
			}

			_isStartedUp = false;

			return !_isStartedUp;
		}

		void ConnectionManager::Tick( float32 elapsed_time )
		{
			if ( !_isStartedUp )
			{
				LOG_ERROR( "[ConnectionManager.%s] ConnectionManager is not started up, ignoring call",
				           THIS_FUNCTION_NAME );
				return;
			}

			for ( auto& it = _pendingConnections.begin(); it != _pendingConnections.end(); ++it )
			{
				PendingConnection& pc = it->second;
				_connectionPipeline->ProcessConnection( pc, *_messageFactory, elapsed_time );

				UpdateTimeout( pc, elapsed_time );
			}
		}

		bool ConnectionManager::DoesPendingConnectionExist( const Address& address ) const
		{
			return ( _pendingConnections.find( address ) != _pendingConnections.end() );
		}

		bool ConnectionManager::ProcessPacket( const Address& address, NetworkPacket& packet )
		{
			if ( !_isStartedUp )
			{
				LOG_ERROR( "[ConnectionManager.%s] ConnectionManager is not started up, ignoring call",
				           THIS_FUNCTION_NAME );
				return false;
			}

			ASSERT( address.IsValid(), "ConnectionManager.%s Address is not valid.", THIS_FUNCTION_NAME );

			bool success = true;
			// Check if pending connection exists
			if ( !DoesPendingConnectionExist( address ) )
			{
				// Try creating a pending connection if it doesn't exist - There have to be empty slots left
				if ( !CreatePendingConnection( address, false ) )
				{
					success = false;

					std::string fullAddress;
					address.GetFull( fullAddress );
					LOG_WARNING( "ConnectionManager.%s Cannot create pending connection with address %s.",
					             THIS_FUNCTION_NAME, fullAddress.c_str() );
				}
			}

			if ( success )
			{
				// Process packet
				PendingConnection& pendingConnection = _pendingConnections[ address ];
				pendingConnection.ProcessPacket( packet );
			}

			return success;
		}

		bool ConnectionManager::CreatePendingConnection( const Address& address, bool started_locally )
		{
			if ( !_isStartedUp )
			{
				LOG_ERROR( "[ConnectionManager.%s] ConnectionManager is not started up, ignoring call",
				           THIS_FUNCTION_NAME );
				return false;
			}

			bool success = false;
			// Check if we have slots left for new pending connections
			if ( AreSlotsAvailableForNewPendingConnection() )
			{
				// Check if we have already created this pending connection
				if ( !DoesPendingConnectionExist( address ) )
				{
					// Create and start up
					_pendingConnections.try_emplace( address, _messageFactory );
					success = _pendingConnections[ address ].StartUp( address, started_locally );
					if ( !success )
					{
						_pendingConnections.erase( address );

						std::string fullAddress;
						address.GetFull( fullAddress );
						LOG_ERROR( "[ConnectionManager.%s] Failed to start up pending connection for address %s.",
						           THIS_FUNCTION_NAME, fullAddress.c_str() );
					}
				}
				else
				{
					std::string fullAddress;
					address.GetFull( fullAddress );
					LOG_WARNING( "ConnectionManager.%s Pending connection with address %s already exists.",
					             THIS_FUNCTION_NAME, fullAddress.c_str() );
				}
			}
			else
			{
				LOG_WARNING( "ConnectionManager.%s Cannot create pending connection. Maximum "
				             "number of concurrent pending connections has been reached.",
				             THIS_FUNCTION_NAME );
			}

			return success;
		}

		bool ConnectionManager::AreSlotsAvailableForNewPendingConnection() const
		{
			const uint32 numberOfAvailableRemotePeerSlots = _remotePeersHandler->GetNumberOfAvailableRemotePeerSlots();
			const uint32 numberOfCurrentPendingConnections = _pendingConnections.size();
			return ( numberOfAvailableRemotePeerSlots > numberOfCurrentPendingConnections );
		}

		bool ConnectionManager::StartConnectingToAddress( const Address& address )
		{
			if ( !_isStartedUp )
			{
				LOG_ERROR( "[ConnectionManager.%s] ConnectionManager is not started up, ignoring call",
				           THIS_FUNCTION_NAME );
				return false;
			}

			bool success = false;
			if ( _canStartConnections )
			{
				if ( CreatePendingConnection( address, true ) )
				{
					PendingConnection& pendingConnection = _pendingConnections[ address ];
					pendingConnection.SetCurrentState( PendingConnectionState::Initializing );
					success = true;
				}
			}
			else
			{
				LOG_WARNING(
				    "ConnectionManager.%s Cannot start connection to address. Starting connections is disabled "
				    "in the configuration.",
				    THIS_FUNCTION_NAME );
			}

			return success;
		}

		void ConnectionManager::GetSuccessConnectionsData(
		    std::vector< SuccessConnectionData >& out_success_connections )
		{
			if ( !_isStartedUp )
			{
				LOG_ERROR( "[ConnectionManager.%s] ConnectionManager is not started up, ignoring call",
				           THIS_FUNCTION_NAME );
				return;
			}

			for ( auto& cit = _pendingConnections.cbegin(); cit != _pendingConnections.cend(); ++cit )
			{
				const PendingConnection& pc = cit->second;
				if ( pc.GetCurrentState() == PendingConnectionState::Completed )
				{
					out_success_connections.emplace_back( pc.GetAddress(), pc.WasStartedLocally(), pc.GetId(),
					                                      pc.GetClientSideId(), pc.GetDataPrefix() );
				}
			}
		}

		void ConnectionManager::RemoveSuccessConnections()
		{
			if ( !_isStartedUp )
			{
				LOG_ERROR( "[ConnectionManager.%s] ConnectionManager is not started up, ignoring call",
				           THIS_FUNCTION_NAME );
				return;
			}

			for ( auto it = _pendingConnections.begin(); it != _pendingConnections.end(); )
			{
				if ( it->second.GetCurrentState() == PendingConnectionState::Completed )
				{
					it = _pendingConnections.erase( it );
				}
				else
				{
					++it;
				}
			}
		}

		void ConnectionManager::GetFailedConnectionsData( std::vector< FailedConnectionData >& out_failed_connections )
		{
			if ( !_isStartedUp )
			{
				LOG_ERROR( "[ConnectionManager.%s] ConnectionManager is not started up, ignoring call",
				           THIS_FUNCTION_NAME );
				return;
			}

			for ( auto& cit = _pendingConnections.cbegin(); cit != _pendingConnections.cend(); ++cit )
			{
				const PendingConnection& pc = cit->second;
				if ( pc.GetCurrentState() == PendingConnectionState::Failed )
				{
					out_failed_connections.emplace_back( pc.GetAddress(), pc.GetConnectionDeniedReason() );
				}
			}
		}

		void ConnectionManager::RemoveFailedConnections()
		{
			if ( !_isStartedUp )
			{
				LOG_ERROR( "[ConnectionManager.%s] ConnectionManager is not started up, ignoring call",
				           THIS_FUNCTION_NAME );
				return;
			}

			for ( auto it = _pendingConnections.begin(); it != _pendingConnections.end(); )
			{
				if ( it->second.GetCurrentState() == PendingConnectionState::Failed )
				{
					it = _pendingConnections.erase( it );
				}
				else
				{
					++it;
				}
			}
		}

		void ConnectionManager::SendData( Socket& socket )
		{
			for ( auto it = _pendingConnections.begin(); it != _pendingConnections.end(); ++it )
			{
				it->second.SendData( socket );
			}
		}

		void ConnectionManager::UpdateTimeout( PendingConnection& pending_connection, float32 elapsed_time )
		{
			pending_connection.UpdateConnectionElapsedTime( elapsed_time );
			if ( pending_connection.GetCurrentConnectionElapsedTime() >= _connectionTimeoutSeconds )
			{
				std::string addressStr;
				pending_connection.GetAddress().GetFull( addressStr );
				LOG_INFO( "ConnectionManager.%s Pending connection with address %s has timed out.", THIS_FUNCTION_NAME,
				          addressStr.c_str() );

				pending_connection.SetCurrentState( PendingConnectionState::Failed );
				pending_connection.SetConnectionDeniedReason( ConnectionFailedReasonType::TIMEOUT );

				if ( _sendDenialOnTimeout )
				{
					// TODO Evaluate if it is worth sending a denial message on timeout since the client-side will also
					// time out its connection (if config stays the same between server and client.)
				}
			}
		}
	} // namespace Connection
} // namespace NetLib
