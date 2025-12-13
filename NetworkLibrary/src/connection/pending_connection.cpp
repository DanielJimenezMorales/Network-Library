#include "pending_connection.h"

#include "logger.h"
#include "asserts.h"

#include "communication/network_packet.h"

namespace NetLib
{
	namespace Connection
	{
		PendingConnection::PendingConnection()
		    : _isStartedUp( false )
		    , _address( Address::GetInvalid() )
		    , _transmissionChannel( nullptr )
		    , _startedLocally( false )
		    , _metricsHandler()
		    , _currentState( PendingConnectionState::Initializing )
		    , _currentConnectionElapsedTimeSeconds( 0.f )
		    , _clientSalt( 0 )
		    , _serverSalt( 0 )
		    , _dataPrefix( 0 )
		    , _hasClientSaltAssigned( false )
		    , _hasServerSaltAssigned( false )
		    , _id( 0 )
		    , _clientSideId( 0 )
		    , _connectionDeniedReason( ConnectionFailedReasonType::UNKNOWN )

		{
		}

		PendingConnection::PendingConnection( MessageFactory* message_factory )
		    : _isStartedUp( false )
		    , _address( Address::GetInvalid() )
		    , _transmissionChannel( message_factory )
		    , _startedLocally( false )
		    , _metricsHandler()
		    , _currentState( PendingConnectionState::Initializing )
		    , _currentConnectionElapsedTimeSeconds( 0.f )
		    , _clientSalt( 0 )
		    , _serverSalt( 0 )
		    , _dataPrefix( 0 )
		    , _hasClientSaltAssigned( false )
		    , _hasServerSaltAssigned( false )
		    , _id( 0 )
		    , _clientSideId( 0 )
		    , _connectionDeniedReason( ConnectionFailedReasonType::UNKNOWN )
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
			_currentConnectionElapsedTimeSeconds = 0.f;
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

		void PendingConnection::SetClientSalt( uint64 client_salt )
		{
			_clientSalt = client_salt;
			_hasClientSaltAssigned = true;
		};

		void PendingConnection::SetServerSalt( uint64 server_salt )
		{
			_serverSalt = server_salt;
			_hasServerSaltAssigned = true;
		};

		bool PendingConnection::AddReceivedMessage( std::unique_ptr< Message > message )
		{
			if ( !_isStartedUp )
			{
				LOG_ERROR( "[PendingConnection.%s] PendingConnection is not started up, ignoring call",
				           THIS_FUNCTION_NAME );
				return false;
			}

			const bool result = _transmissionChannel.AddReceivedMessage( std::move( message ), _metricsHandler );

			return result;
		}

		void PendingConnection::SendData( Socket& socket )
		{
			_transmissionChannel.CreateAndSendPacket( socket, _address, _metricsHandler );
		}

		void PendingConnection::UpdateConnectionElapsedTime( float32 elapsed_time )
		{
			_currentConnectionElapsedTimeSeconds += elapsed_time;
		};
	} // namespace Connection
} // namespace NetLib
