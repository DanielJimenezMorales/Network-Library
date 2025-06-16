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
	    , _currentState( ClientState::CS_Disconnected )
	    , _replicationMessagesProcessor()
	    , _clientIndex( 0 )
	    , _timeSyncer()
	{
	}

	Client::~Client()
	{
	}

	bool Client::StartClient( const std::string& server_ip, uint32 server_port )
	{
		return Start( server_ip, 0 ); // Port is zero so the system picks up a random port number
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

	bool Client::StartConcrete( const std::string& ip, uint32 port )
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
			case MessageType::PingPong:
				{
					break;
				}
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
			RemotePeer* serverRemotePeer = _remotePeersHandler.GetRemotePeerFromAddress( _serverAddress );
			if ( serverRemotePeer == nullptr )
			{
				LOG_ERROR( "There is no Remote peer corresponding to IP: %s", _serverAddress.GetIP() );
			}
			else
			{
				_timeSyncer.Update( elapsedTime, *serverRemotePeer );
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

		// TODO Do not hardcode it like this. It might looks weird
		_replicationMessagesProcessor.SetLocalClientId( _clientIndex );

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
		_timeSyncer.ProcessTimeResponse( message );
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

	void Client::OnServerDisconnect()
	{
		LOG_INFO( "ON SERVER DISCONNECT" );
		Stop();
	}
} // namespace NetLib
