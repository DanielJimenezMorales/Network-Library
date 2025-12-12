#include "pending_connection.h"

#include "logger.h"
#include "asserts.h"

#include "communication/network_packet.h"

namespace NetLib
{
	PendingConnection::PendingConnection()
	    : _isStartedUp( false )
	    , _address( Address::GetInvalid() )
	    , _transmissionChannel( nullptr )
	    , _startedLocally( false )
	{
	}

	PendingConnection::PendingConnection( MessageFactory* message_factory )
	    : _isStartedUp( false )
	    , _address( Address::GetInvalid() )
	    , _transmissionChannel( message_factory )
	    , _startedLocally( false )
	{
	}

	bool PendingConnection::StartUp( const Address& address, bool started_locally )
	{
		if ( _isStartedUp )
		{
			LOG_ERROR( "[PendingConnection.%s] PendingConnection is already started up, ignoring call",
			           THIS_FUNCTION_NAME );
			return false;
		}

		_address = address;
		_startedLocally = started_locally;
		_currentState = PendingConnectionState::Initializing;
		_hasClientSaltAssigned = false;
		_hasServerSaltAssigned = false;
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

	void PendingConnection::ProcessPacket( NetworkPacket& packet )
	{
		// Process packet ACKs
		/*ASSERT( packet.GetHeader().channelType != TransmissionChannelType::ReliableOrdered,
		        "Pending connection packets must be reliable ordered." );*/

		const uint32 acks = packet.GetHeader().ackBits;
		const uint16 lastAckedMessageSequenceNumber = packet.GetHeader().lastAckedSequenceNumber;
		_transmissionChannel.ProcessACKs( acks, lastAckedMessageSequenceNumber, _metricsHandler );

		// Process packet messages one by one
		while ( packet.GetNumberOfMessages() > 0 )
		{
			std::unique_ptr< Message > message = packet.TryGetNextMessage();
			AddReceivedMessage( std::move( message ) );
		}

		if ( _metricsHandler.HasMetric( Metrics::MetricType::DOWNLOAD_BANDWIDTH ) )
		{
			const uint32 packet_size = packet.Size();
			_metricsHandler.AddValue( Metrics::MetricType::DOWNLOAD_BANDWIDTH, packet_size );
		}
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
