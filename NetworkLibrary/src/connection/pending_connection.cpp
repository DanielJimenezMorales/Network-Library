#include "pending_connection.h"

#include "logger.h"

namespace NetLib
{
	PendingConnection::PendingConnection()
	    : _isStartedUp( false )
	    , _address( Address::GetInvalid() )
	    , _transmissionChannel( nullptr )
	{
	}

	PendingConnection::PendingConnection( MessageFactory* message_factory )
	    : _isStartedUp( false )
	    , _address( Address::GetInvalid() )
	    , _transmissionChannel( message_factory )
	{
	}

	bool PendingConnection::StartUp( const Address& address )
	{
		if ( _isStartedUp )
		{
			LOG_ERROR( "[PendingConnection.%s] PendingConnection is already started up, ignoring call",
			           THIS_FUNCTION_NAME );
			return false;
		}

		_address = address;
		_currentState = PendingConnectionState::Initializing;
		_isStartedUp = true;
		return true;
	}

	bool PendingConnection::ShutDown()
	{
		if ( !_isStartedUp )
		{
			LOG_ERROR( "[PendingConnection.%s] PendingConnection is not started up, ignoring call",
			           THIS_FUNCTION_NAME );
			return false;
		}

		_address = Address::GetInvalid();
		_isStartedUp = false;
		return true;
	}

	const Message* PendingConnection::GetPendingReadyToProcessMessage()
	{
		if ( !_isStartedUp )
		{
			LOG_ERROR( "[PendingConnection.%s] PendingConnection is not started up, ignoring call",
			           THIS_FUNCTION_NAME );
			return nullptr;
		}

		const Message* message = nullptr;

		if ( _transmissionChannel.ArePendingReadyToProcessMessages() )
		{
			message = _transmissionChannel.GetReadyToProcessMessage();
		}

		return message;
	}

	bool PendingConnection::AddMessage( std::unique_ptr< Message > message )
	{
		if ( !_isStartedUp )
		{
			LOG_ERROR( "[PendingConnection.%s] PendingConnection is not started up, ignoring call",
			           THIS_FUNCTION_NAME );
			return false;
		}

		return _transmissionChannel.AddMessageToSend( std::move( message ) );
	}

	bool PendingConnection::AddReceivedMessage( std::unique_ptr< Message > message )
	{
		if ( !_isStartedUp )
		{
			LOG_ERROR( "[PendingConnection.%s] PendingConnection is not started up, ignoring call",
			           THIS_FUNCTION_NAME );
			return false;
		}

		const bool result = _transmissionChannel.AddReceivedMessage( std::move( message ), _metricsHandler );
		// TODO Reset inactivity

		return result;
	}

	void PendingConnection::SendData( Socket& socket )
	{
		_transmissionChannel.CreateAndSendPacket( socket, _address, _metricsHandler );
	}
} // namespace NetLib
