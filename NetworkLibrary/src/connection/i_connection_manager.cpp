#include "i_connection_manager.h"

#include "connection/i_connection_pipeline.h"

#include "logger.h"
#include "asserts.h"

namespace NetLib
{
	ConnectionManager::ConnectionManager( MessageFactory* message_factory )
	    : _isStartedUp( false )
	    , _pendingConnections()
	    , _connectionPipeline( nullptr )
	    , _messageFactory( message_factory )
	{
	}

	bool ConnectionManager::StartUp( ConnectionConfiguration& configuration )
	{
		if ( _isStartedUp )
		{
			LOG_ERROR( "[ConnectionManager.%s] ConnectionManager is already started up, ignoring call",
			           THIS_FUNCTION_NAME );
			return false;
		}

		if ( configuration.connectionPipeline != nullptr )
		{
			_connectionPipeline = configuration.connectionPipeline;
			_maxPendingConnections = configuration.maxPendingConnections;
			_canStartConnections = configuration.canStartConnections;

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
			_connectionPipeline->ProcessConnection( it->second, *_messageFactory, elapsed_time );
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

		return true;
	}

	bool ConnectionManager::AddIncomingMessageToPendingConnection( const Address& address,
	                                                               std::unique_ptr< Message > message )
	{
		ASSERT( message != nullptr, "Message can't be null" );

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
			pendingConnection.AddMessage( std::move( message ) );
			success = true;
		}
		else
		{
			// We only accept connection requests for untracked addresses
			if ( message->GetHeader().type == MessageType::ConnectionRequest )
			{
				// Check if pending connection creation is successful - There have to be empty slots left
				if ( CreatePendingConnection( address ) )
				{
					PendingConnection& pendingConnection = _pendingConnections[ address ];
					pendingConnection.AddMessage( std::move( message ) );
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
			else
			{
				std::string fullAddress;
				address.GetFull( fullAddress );
				LOG_WARNING( "ConnectionManager.%s Cannot create pending connection with address %s using a message "
				             "different that a connection request. Ignoring call.",
				             THIS_FUNCTION_NAME, fullAddress.c_str() );
			}
		}

		return success;
	}

	bool ConnectionManager::CreatePendingConnection( const Address& address )
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
				_pendingConnections.emplace( address, PendingConnection( _messageFactory ) );
				success = _pendingConnections[ address ].StartUp( address );
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
			if ( CreatePendingConnection( address ) )
			{
				PendingConnection& pendingConnection = _pendingConnections[ address ];
				pendingConnection.SetCurrentState( PendingConnectionState::Initializing );
				success = true;
			}
		}
		else
		{
			LOG_WARNING( "ConnectionManager.%s Cannot start connection to address. Starting connections is disabled "
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
				out_connected_pending_connections.emplace_back( pc.GetAddress(), pc.GetId(), pc.GetDataPrefix() );
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
} // namespace NetLib
