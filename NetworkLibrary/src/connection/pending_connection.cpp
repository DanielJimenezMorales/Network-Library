#include "pending_connection.h"

namespace NetLib
{
	PendingConnection::PendingConnection()
	    : _address( Address::GetInvalid() )
	    , _transmissionChannel( nullptr )
	{
	}

	PendingConnection::PendingConnection( MessageFactory* message_factory )
	    : _address( Address::GetInvalid() )
	    , _transmissionChannel( message_factory )
	{
	}

	bool PendingConnection::StartUp( const Address& address )
	{
		// TODO
		return false;
	}

	bool PendingConnection::ShutDown()
	{
		// TODO
		return false;
	}

	const Message* PendingConnection::GetPendingReadyToProcessMessage()
	{
		const Message* message = nullptr;

		if ( _transmissionChannel.ArePendingReadyToProcessMessages() )
		{
			message = _transmissionChannel.GetReadyToProcessMessage();
		}

		return message;
	}

	bool PendingConnection::AddMessage( std::unique_ptr< Message > message )
	{
		return _transmissionChannel.AddMessageToSend( std::move( message ) );
	}

	bool PendingConnection::AddReceivedMessage( std::unique_ptr< Message > message )
	{
		bool result = false;

		_transmissionChannel.AddReceivedMessage( std::move( message ), _metricsHandler );
		// TODO Reset inactivity

		return result;
	}
} // namespace NetLib
