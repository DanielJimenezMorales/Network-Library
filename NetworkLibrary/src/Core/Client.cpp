#include "client.h"

#include <memory>

#include "logger.h"

#include "core/remote_peer.h"
#include "core/time_clock.h"

#include "inputs/i_input_state.h"

#include "communication/message.h"
#include "communication/message_factory.h"
#include "communication/network_packet.h"

namespace NetLib
{
	Client::Client( float32 serverMaxInactivityTimeout )
	    : Peer( PeerType::CLIENT, 1, 1024, 1024 )
	    , _serverAddress( "127.0.0.1", 54000 )
	    , inGameMessageID( 0 )
	    , _timeSinceLastTimeRequest( 0.0f )
	    , _numberOfInitialTimeRequestBurstLeft( NUMBER_OF_INITIAL_TIME_REQUESTS_BURST )
	    , _currentState( ClientState::CS_Disconnected )
	    , _replicationMessagesProcessor( &_networkEntityFactoryRegistry )
	    , _clientIndex( 0 )
	{
	}

	Client::~Client()
	{
	}

	void Client::SendInputs( const IInputState& inputState )
	{
		RemotePeer* serverPeer = _remotePeersHandler.GetRemotePeerFromAddress( _serverAddress );
		if ( serverPeer == nullptr )
		{
			LOG_ERROR( "Can not send inputs to server because server peer hasn't been found." );
			return;
		}

		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr< Message > message = messageFactory.LendMessage( MessageType::Inputs );
		message->SetOrdered( false );
		message->SetReliability( false );
		std::unique_ptr< InputStateMessage > inputsMessage( static_cast< InputStateMessage* >( message.release() ) );

		int32 dataSize = inputState.GetSize();
		uint8* data = new uint8[ dataSize ];
		Buffer inputDataBuffer( data, dataSize );
		inputState.Serialize( inputDataBuffer );
		inputsMessage->dataSize = dataSize;
		inputsMessage->data = data;

		serverPeer->AddMessage( std::move( inputsMessage ) );

		LOG_INFO( "Input state message created" );
	}

	uint32 Client::GetLocalClientId() const
	{
		if ( GetConnectionState() != PeerConnectionState::PCS_Connected )
		{
			LOG_WARNING( "Can not get the local client ID if the peer is not connected" );
			return 0;
		}

		return _clientIndex;
	}

	bool Client::StartConcrete()
	{
		BindSocket( Address( "127.0.0.1", 0 ) ); // Port is zero so the system picks up a random port number

		_currentState = ClientState::CS_SendingConnectionRequest;

		uint64 clientSalt = GenerateClientSaltNumber();
		AddRemotePeer( _serverAddress, 0, clientSalt, 0 );

		SubscribeToOnRemotePeerDisconnect(
		    [ this ]( uint32 )
		    {
			    OnServerDisconnect();
		    } );

		LOG_INFO( "Client started succesfully!" );

		return true;
	}

	uint64 Client::GenerateClientSaltNumber()
	{
		// TODO Change this for a better generator. rand is not generating a full 64bit integer since its maximum is
		// roughly 32767. I have tried to use mt19937_64 but I think I get a conflict with winsocks and
		// std::uniform_int_distribution
		srand( time( NULL ) );
		return rand();
	}

	void Client::ProcessMessageFromPeer( const Message& message, RemotePeer& remotePeer )
	{
		MessageType messageType = message.GetHeader().type;

		switch ( messageType )
		{
			case MessageType::ConnectionChallenge:
				if ( _currentState == ClientState::CS_SendingConnectionRequest ||
				     _currentState == ClientState::CS_SendingConnectionChallengeResponse )
				{
					const ConnectionChallengeMessage& connectionChallengeMessage =
					    static_cast< const ConnectionChallengeMessage& >( message );
					ProcessConnectionChallenge( connectionChallengeMessage, remotePeer );
				}
				break;
			case MessageType::ConnectionAccepted:
				if ( _currentState == ClientState::CS_SendingConnectionChallengeResponse )
				{
					const ConnectionAcceptedMessage& connectionAcceptedMessage =
					    static_cast< const ConnectionAcceptedMessage& >( message );
					ProcessConnectionRequestAccepted( connectionAcceptedMessage, remotePeer );
				}
				break;
			case MessageType::ConnectionDenied:
				if ( _currentState == ClientState::CS_SendingConnectionChallengeResponse ||
				     _currentState == ClientState::CS_SendingConnectionRequest )
				{
					const ConnectionDeniedMessage& connectionDeniedMessage =
					    static_cast< const ConnectionDeniedMessage& >( message );
					ProcessConnectionRequestDenied( connectionDeniedMessage );
				}
				break;
			case MessageType::Disconnection:
				if ( _currentState == ClientState::CS_Connected )
				{
					const DisconnectionMessage& disconnectionMessage =
					    static_cast< const DisconnectionMessage& >( message );
					ProcessDisconnection( disconnectionMessage, remotePeer );
				}
				break;
			case MessageType::TimeResponse:
				if ( _currentState == ClientState::CS_Connected )
				{
					const TimeResponseMessage& timeResponseMessage =
					    static_cast< const TimeResponseMessage& >( message );
					ProcessTimeResponse( timeResponseMessage );
				}
				break;
			case MessageType::Replication:
				if ( _currentState == ClientState::CS_Connected )
				{
					const ReplicationMessage& replicationMessage = static_cast< const ReplicationMessage& >( message );
					ProcessReplicationAction( replicationMessage );
				}
				break;
			default:
				LOG_WARNING( "Invalid Message type, ignoring it..." );
				break;
		}
	}

	void Client::ProcessMessageFromUnknownPeer( const Message& message, const Address& address )
	{
		LOG_WARNING( "Client does not process messages from unknown peers. Ignoring it..." );
	}

	void Client::TickConcrete( float32 elapsedTime )
	{
		if ( _currentState == ClientState::CS_SendingConnectionRequest ||
		     _currentState == ClientState::CS_SendingConnectionChallengeResponse )
		{
			RemotePeer* remotePeer = _remotePeersHandler.GetRemotePeerFromAddress( _serverAddress );
			if ( remotePeer == nullptr )
			{
				LOG_ERROR( "Can't create new Connection Request Message because there is no remote peer corresponding "
				           "to IP: %s",
				           _serverAddress.GetIP() );
				return;
			}

			CreateConnectionRequestMessage( *remotePeer );
		}

		if ( _currentState == ClientState::CS_Connected )
		{
			UpdateTimeRequestsElapsedTime( elapsedTime );

			RemotePeer* remotePeer = _remotePeersHandler.GetRemotePeerFromAddress( _serverAddress );
			if ( remotePeer == nullptr )
			{
				LOG_ERROR( "There is no Remote peer corresponding to IP: %s", _serverAddress.GetIP() );
				return;
			}
		}
	}

	bool Client::StopConcrete()
	{
		_currentState = ClientState::CS_Disconnected;
		return true;
	}

	void Client::ProcessConnectionChallenge( const ConnectionChallengeMessage& message, RemotePeer& remotePeer )
	{
		LOG_INFO( "Challenge packet received from server" );

		uint64 clientSalt = message.clientSalt;
		uint64 serverSalt = message.serverSalt;
		if ( remotePeer.GetClientSalt() != clientSalt )
		{
			LOG_WARNING( "The generated salt number does not match the server's challenge client salt number. Aborting "
			             "operation" );
			return;
		}

		remotePeer.SetServerSalt( serverSalt );

		_currentState = ClientState::CS_SendingConnectionChallengeResponse;

		CreateConnectionChallengeResponse( remotePeer );

		LOG_INFO( "Sending challenge response packet to server..." );
	}

	void Client::ProcessConnectionRequestAccepted( const ConnectionAcceptedMessage& message, RemotePeer& remotePeer )
	{
		if ( remotePeer.GeturrentState() == RemotePeerState::Connected )
		{
			LOG_INFO( "The server's remote peer is already connected. Ignoring message" );
			return;
		}

		uint64 remoteDataPrefix = message.prefix;
		if ( remoteDataPrefix != remotePeer.GetDataPrefix() )
		{
			LOG_WARNING( "Packet prefix does not match. Skipping packet..." );
			return;
		}

		ConnectRemotePeer( remotePeer );

		_clientIndex = message.clientIndexAssigned;
		_currentState = ClientState::CS_Connected;

		LOG_INFO( "Connection accepted!" );
		ExecuteOnLocalPeerConnect();
	}

	void Client::ProcessConnectionRequestDenied( const ConnectionDeniedMessage& message )
	{
		LOG_INFO( "Processing connection denied" );
		ConnectionFailedReasonType reason = static_cast< ConnectionFailedReasonType >( message.reason );

		RequestStop( false, reason );
	}

	void Client::ProcessDisconnection( const DisconnectionMessage& message, RemotePeer& remotePeer )
	{
		uint64 dataPrefix = message.prefix;
		if ( dataPrefix != remotePeer.GetDataPrefix() )
		{
			LOG_WARNING( "Packet prefix does not match. Skipping message..." );
			return;
		}

		LOG_INFO( "Disconnection message received from server with reason code equal to %hhu. Disconnecting...",
		          message.reason );

		StartDisconnectingRemotePeer( remotePeer.GetClientIndex(), false, ConnectionFailedReasonType::CFR_UNKNOWN );
	}

	void Client::ProcessTimeResponse( const TimeResponseMessage& message )
	{
		LOG_INFO( "PROCESSING TIME RESPONSE" );

		// Add new RTT to buffer
		TimeClock& timeClock = TimeClock::GetInstance();
		uint32 rtt = timeClock.GetLocalTimeMilliseconds() - message.remoteTime;
		_timeRequestRTTs.push_back( rtt );

		if ( _timeRequestRTTs.size() == TIME_REQUEST_RTT_BUFFER_SIZE + 1 )
		{
			_timeRequestRTTs.pop_front();
		}

		// Get RTT to adjust server's clock elapsed time
		uint32 meanRTT = 0;
		if ( _timeRequestRTTs.size() == TIME_REQUEST_RTT_BUFFER_SIZE )
		{
			// Sort RTTs and remove the smallest and biggest values (They are considered outliers!)
			std::list< uint32 > sortedTimeRequestRTTs = _timeRequestRTTs;
			sortedTimeRequestRTTs.sort();

			// Remove potential outliers
			for ( uint32 i = 0; i < NUMBER_OF_RTTS_CONSIDERED_OUTLIERS_PER_SIDE; ++i )
			{
				sortedTimeRequestRTTs.pop_back();
				sortedTimeRequestRTTs.pop_front();
			}

			std::list< uint32 >::const_iterator cit = sortedTimeRequestRTTs.cbegin();
			for ( ; cit != sortedTimeRequestRTTs.cend(); ++cit )
			{
				meanRTT += *cit;
			}

			const uint32 NUMBER_OF_VALID_RTT_TO_AVERAGE =
			    TIME_REQUEST_RTT_BUFFER_SIZE - ( 2 * NUMBER_OF_RTTS_CONSIDERED_OUTLIERS_PER_SIDE );

			meanRTT /= NUMBER_OF_VALID_RTT_TO_AVERAGE;
		}
		else
		{
			meanRTT = rtt;
		}

		// Calculate server clock delta time
		uint32 serverClockElapsedTimeMilliseconds = message.serverTime - message.remoteTime - ( meanRTT / 2 );
		float64 serverClockElapsedTimeSeconds = static_cast< float64 >( serverClockElapsedTimeMilliseconds ) / 1000;
		timeClock.SetServerClockTimeDelta( serverClockElapsedTimeSeconds );

		LOG_INFO( "SERVER TIME UPDATED. Local time: %f sec, Server time: %f sec", timeClock.GetLocalTimeSeconds(),
		          timeClock.GetServerTimeSeconds() );
	}

	void Client::ProcessReplicationAction( const ReplicationMessage& message )
	{
		_replicationMessagesProcessor.Client_ProcessReceivedReplicationMessage( message );
	}

	void Client::CreateConnectionRequestMessage( RemotePeer& remotePeer )
	{
		// Get a connection request message
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr< Message > message = messageFactory.LendMessage( MessageType::ConnectionRequest );

		if ( message == nullptr )
		{
			LOG_ERROR(
			    "Can't create new Connection Request Message because the MessageFactory has returned a null message" );
			return;
		}

		std::unique_ptr< ConnectionRequestMessage > connectionRequestMessage(
		    static_cast< ConnectionRequestMessage* >( message.release() ) );

		// Set connection request fields
		connectionRequestMessage->clientSalt = remotePeer.GetClientSalt();

		// Store message in server's pending connection in order to send it
		remotePeer.AddMessage( std::move( connectionRequestMessage ) );

		LOG_INFO( "Connection request created." );
	}

	void Client::CreateConnectionChallengeResponse( RemotePeer& remotePeer )
	{
		// Get a connection challenge message
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr< Message > message = messageFactory.LendMessage( MessageType::ConnectionChallengeResponse );
		if ( message == nullptr )
		{
			LOG_ERROR( "Can't create new Connection Challenge Response Message because the MessageFactory has returned "
			           "a null message" );
			return;
		}

		std::unique_ptr< ConnectionChallengeResponseMessage > connectionChallengeResponseMessage(
		    static_cast< ConnectionChallengeResponseMessage* >( message.release() ) );

		// Set connection challenge fields
		connectionChallengeResponseMessage->prefix = remotePeer.GetDataPrefix();

		// Store message in server's pending connection in order to send it
		remotePeer.AddMessage( std::move( connectionChallengeResponseMessage ) );
	}

	void Client::CreateTimeRequestMessage( RemotePeer& remotePeer )
	{
		LOG_INFO( "TIME REQUEST CREATED" );
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr< Message > lendMessage( messageFactory.LendMessage( MessageType::TimeRequest ) );

		std::unique_ptr< TimeRequestMessage > timeRequestMessage(
		    static_cast< TimeRequestMessage* >( lendMessage.release() ) );

		timeRequestMessage->SetOrdered( true );
		TimeClock& timeClock = TimeClock::GetInstance();
		timeRequestMessage->remoteTime = timeClock.GetLocalTimeMilliseconds();

		remotePeer.AddMessage( std::move( timeRequestMessage ) );
	}

	void Client::UpdateTimeRequestsElapsedTime( float32 elapsedTime )
	{
		if ( _numberOfInitialTimeRequestBurstLeft > 0 )
		{
			RemotePeer* remotePeer = _remotePeersHandler.GetRemotePeerFromAddress( _serverAddress );
			if ( remotePeer == nullptr )
			{
				LOG_ERROR( "There is no Remote peer corresponding to IP: %s", _serverAddress.GetIP().c_str() );
				return;
			}

			--_numberOfInitialTimeRequestBurstLeft;
			CreateTimeRequestMessage( *remotePeer );
			return;
		}

		_timeSinceLastTimeRequest += elapsedTime;
		if ( _timeSinceLastTimeRequest >= TIME_REQUESTS_FREQUENCY_SECONDS )
		{
			RemotePeer* remotePeer = _remotePeersHandler.GetRemotePeerFromAddress( _serverAddress );
			if ( remotePeer == nullptr )
			{
				LOG_ERROR( "There is no Remote peer corresponding to IP: %s", _serverAddress.GetIP().c_str() );
				return;
			}

			_timeSinceLastTimeRequest = 0;
			CreateTimeRequestMessage( *remotePeer );
		}
	}

	void Client::OnServerDisconnect()
	{
		LOG_INFO( "ON SERVER DISCONNECT" );
		Stop();
	}
} // namespace NetLib
