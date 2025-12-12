#include "connection_manager.h"

#include "connection/i_connection_pipeline.h"
#include "communication/network_packet.h"
#include "communication/message.h"

#include "logger.h"
#include "asserts.h"

namespace NetLib
{
	namespace Connection
	{
		ConnectionManager::ConnectionManager()
		    : _isStartedUp( false )
		    , _pendingConnections()
		    , _connectionPipeline( nullptr )
		    , _messageFactory( nullptr )
		{
		}

		bool ConnectionManager::StartUp( ConnectionConfiguration& configuration, MessageFactory* message_factory )
		{
			if ( _isStartedUp )
			{
				LOG_ERROR( "[ConnectionManager.%s] ConnectionManager is already started up, ignoring call",
				           THIS_FUNCTION_NAME );
				return false;
			}

			ASSERT( message_factory != nullptr, "Message factory can't be null" );
			_messageFactory = message_factory;

			if ( configuration.connectionPipeline != nullptr )
			{
				_connectionPipeline = configuration.connectionPipeline;
				_maxPendingConnections = configuration.maxPendingConnections;
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
			if ( !_isStartedUp )
			{
				LOG_ERROR( "[ConnectionManager.%s] ConnectionManager is not started up, ignoring call",
				           THIS_FUNCTION_NAME );
				return false;
			}

			bool found = false;
			if ( _pendingConnections.find( address ) != _pendingConnections.end() )
			{
				found = true;
			}

			return found;
		}

		bool ConnectionManager::ProcessPacket( const Address& address, NetworkPacket& packet )
		{
			if ( !_isStartedUp )
			{
				LOG_ERROR( "[ConnectionManager.%s] ConnectionManager is not started up, ignoring call",
				           THIS_FUNCTION_NAME );
				return false;
			}

			bool success = false;
			if ( DoesPendingConnectionExist( address ) )
			{
				PendingConnection& pendingConnection = _pendingConnections[ address ];
				pendingConnection.ProcessPacket( packet );
				success = true;
			}
			else
			{
				// Check if pending connection creation is successful - There have to be empty slots left
				if ( CreatePendingConnection( address, false ) )
				{
					PendingConnection& pendingConnection = _pendingConnections[ address ];
					pendingConnection.ProcessPacket( packet );
					success = true;
				}
				else
				{
					std::string fullAddress;
					address.GetFull( fullAddress );
					LOG_WARNING( "ConnectionManager.%s Cannot create pending connection with address %s.",
					             THIS_FUNCTION_NAME, fullAddress.c_str() );
				}
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
			if ( _pendingConnections.size() < _maxPendingConnections )
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

		void ConnectionManager::GetConnectedPendingConnectionsData(
		    std::vector< PendingConnectionData >& out_connected_pending_connections )
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
					out_connected_pending_connections.emplace_back( pc.GetAddress(), pc.WasStartedLocally(), pc.GetId(),
					                                                pc.GetClientSideId(), pc.GetDataPrefix() );
				}
			}
		}

		void ConnectionManager::ClearConnectedPendingConnections()
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

		void ConnectionManager::GetDeniedPendingConnectionsData(
		    std::vector< PendingConnectionFailedData >& out_denied_pending_connections )
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
					out_denied_pending_connections.emplace_back( pc.GetAddress(), pc.GetConnectionDeniedReason() );
				}
			}
		}

		void ConnectionManager::ClearDeniedPendingConnections()
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

		void ConnectionManager::SendDataToPendingConnections( Socket& socket )
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
				pending_connection.SetConnectionDeniedReason( ConnectionFailedReasonType::CFR_TIMEOUT );

				if ( _sendDenialOnTimeout )
				{
					// TODO Evaluate if it is worth sending a denial message on timeout since the client-side will also
					// time out its connection (if config stays the same between server and client.)
				}
			}
		}
	} // namespace Connection
} // namespace NetLib
