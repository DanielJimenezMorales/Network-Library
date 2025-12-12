#include "client.h"

#include <memory>

#include "logger.h"
#include "asserts.h"

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
	    , _replicationMessagesProcessor()
	    , _clientIndex( 0 )
	    , _timeSyncer()
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

		std::unique_ptr< Message > message = _messageFactory.LendMessage( MessageType::Inputs );
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
		if ( GetConnectionState() != PeerConnectionState::Connected )
		{
			LOG_WARNING( "Can not get the local client ID if the peer is not connected" );
			return 0;
		}

		return _clientIndex;
	}

	bool Client::StartConcrete( const std::string& ip, uint32 port )
	{
		BindSocket( Address( "127.0.0.1", 0 ) ); // Port is zero so the system picks up a random port number

		_connectionManager.StartConnectingToAddress( _serverAddress );

		SubscribeToOnRemotePeerDisconnect(
		    [ this ]( uint32 )
		    {
			    OnServerDisconnect();
		    } );

		LOG_INFO( "Client started succesfully!" );

		return true;
	}

	void Client::ProcessMessageFromPeer( const Message& message, RemotePeer& remotePeer )
	{
		MessageType messageType = message.GetHeader().type;

		switch ( messageType )
		{
			case MessageType::Disconnection:
				if ( GetConnectionState() == PeerConnectionState::Connected )
				{
					const DisconnectionMessage& disconnectionMessage =
					    static_cast< const DisconnectionMessage& >( message );
					ProcessDisconnection( disconnectionMessage, remotePeer );
				}
				break;
			case MessageType::TimeResponse:
				if ( GetConnectionState() == PeerConnectionState::Connected )
				{
					const TimeResponseMessage& timeResponseMessage =
					    static_cast< const TimeResponseMessage& >( message );
					ProcessTimeResponse( timeResponseMessage );
				}
				break;
			case MessageType::Replication:
				if ( GetConnectionState() == PeerConnectionState::Connected )
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

	void Client::TickConcrete( float32 elapsedTime )
	{
		if ( GetConnectionState() == PeerConnectionState::Connected )
		{
			RemotePeer* serverRemotePeer = _remotePeersHandler.GetRemotePeerFromAddress( _serverAddress );
			if ( serverRemotePeer == nullptr )
			{
				LOG_ERROR( "There is no Remote peer corresponding to IP: %s", _serverAddress.GetIP() );
			}
			else
			{
				_timeSyncer.Update( elapsedTime, *serverRemotePeer, _messageFactory );
			}
		}
	}

	bool Client::StopConcrete()
	{
		return true;
	}

	void Client::OnPendingConnectionAccepted( const Connection::SuccessConnectionData& data )
	{
		ASSERT( data.startedLocally,
		        "Client-side can't receive a connection accepted apart from the one that was started locally." );

		_clientIndex = data.clientSideId;

		// TODO Do not hardcode it like this. It might looks weird
		_replicationMessagesProcessor.SetLocalClientId( _clientIndex );

		LOG_INFO( "Connection accepted!" );
		ExecuteOnLocalPeerConnect();
	}

	void Client::OnPendingConnectionDenied( const Connection::FailedConnectionData& data )
	{
		LOG_INFO( "Client.%s Connection denied. Reason: %u", THIS_FUNCTION_NAME, static_cast< uint8 >( data.reason ) );
		RequestStop( false, data.reason );
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

		StartDisconnectingRemotePeer( remotePeer.GetClientIndex(), false,
		                              Connection::ConnectionFailedReasonType::CFR_UNKNOWN );
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

	void Client::OnServerDisconnect()
	{
		LOG_INFO( "ON SERVER DISCONNECT" );
		Stop();
	}
} // namespace NetLib
