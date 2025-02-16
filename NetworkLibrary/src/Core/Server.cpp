#include "server.h"

#include <cassert>
#include <memory>

#include "logger.h"

#include "core/time_clock.h"

#include "inputs/i_input_state.h"
#include "inputs/i_input_state_factory.h"

#include "communication/message.h"
#include "communication/message_factory.h"
#include "communication/network_packet.h"

#include "replication/replication_manager.h"

#define SERVER_PORT 54000

namespace NetLib
{
	Server::Server( int32 maxConnections )
	    : Peer( PeerType::SERVER, maxConnections, 1024, 1024 )
	    , _remotePeerInputsHandler()
	    , _replicationManager()
	{
	}

	uint32 Server::CreateNetworkEntity( uint32 entityType, uint32 controlledByPeerId, float32 posX, float32 posY )
	{
		if ( GetConnectionState() != PeerConnectionState::PCS_Connected )
		{
			LOG_WARNING( "Can't create Network entity of type %d because the server is not connected.",
			             static_cast< int >( entityType ) );
			return 0;
		}

		return _replicationManager.CreateNetworkEntity( entityType, controlledByPeerId, posX, posY );
	}

	void Server::DestroyNetworkEntity( uint32 entityId )
	{
		if ( GetConnectionState() != PeerConnectionState::PCS_Connected )
		{
			LOG_WARNING( "Can't destroy Network entity with ID: %d because the server is not connected.",
			             static_cast< int >( entityId ) );
			return;
		}

		_replicationManager.RemoveNetworkEntity( entityId );
	}

	void Server::RegisterInputStateFactory( IInputStateFactory* factory )
	{
		// TODO Create a method for releasing all the inputs consumed during the current tick
		assert( factory != nullptr );
		_inputsFactory = factory;
	}

	const IInputState* Server::GetInputFromRemotePeer( uint32 remotePeerId )
	{
		return _remotePeerInputsHandler.GetNextInputFromRemotePeer( remotePeerId );
	}

	Server::~Server()
	{
	}

	bool Server::StartConcrete()
	{
		BindSocket( Address( IPV4_ANY, SERVER_PORT ) );
		LOG_INFO( "Server started succesfully!" );

		ExecuteOnLocalPeerConnect();

		SubscribeToOnRemotePeerDisconnect(
		    std::bind( &Server::RemoveReplicationEntitiesControlledByPeer, this, std::placeholders::_1 ) );
		return true;
	}

	void Server::TickConcrete( float32 elapsedTime )
	{
		TickReplication();
	}

	uint64 Server::GenerateServerSalt() const
	{
		// TODO Change this in order to get another random generator that generates 64bit numbers
		srand( time( NULL ) + 3589 );
		uint64 serverSalt = rand();
		return serverSalt;
	}

	void Server::ProcessMessageFromPeer( const Message& message, RemotePeer& remotePeer )
	{
		MessageType messageType = message.GetHeader().type;
		LOG_INFO( "Me proceso type %d", static_cast< int >( messageType ) );

		switch ( messageType )
		{
			case MessageType::ConnectionRequest:
				{
					const ConnectionRequestMessage& connectionRequestMessage =
					    static_cast< const ConnectionRequestMessage& >( message );
					ProcessConnectionRequest( connectionRequestMessage, remotePeer.GetAddress() );
					break;
				}
			case MessageType::ConnectionChallengeResponse:
				{
					const ConnectionChallengeResponseMessage& connectionChallengeResponseMessage =
					    static_cast< const ConnectionChallengeResponseMessage& >( message );
					ProcessConnectionChallengeResponse( connectionChallengeResponseMessage, remotePeer );
					break;
				}
			case MessageType::TimeRequest:
				{
					const TimeRequestMessage& timeRequestMessage = static_cast< const TimeRequestMessage& >( message );
					ProcessTimeRequest( timeRequestMessage, remotePeer );
					break;
				}
			case MessageType::Disconnection:
				{
					const DisconnectionMessage& disconnectionMessage =
					    static_cast< const DisconnectionMessage& >( message );
					ProcessDisconnection( disconnectionMessage, remotePeer );
					break;
				}
			case MessageType::Inputs:
				{
					const InputStateMessage& inputsMessage = static_cast< const InputStateMessage& >( message );
					ProcessInputs( inputsMessage, remotePeer );
					break;
				}
			default:
				LOG_WARNING( "Invalid Message type, ignoring it..." );
				break;
		}
	}

	void Server::ProcessMessageFromUnknownPeer( const Message& message, const Address& address )
	{
		if ( message.GetHeader().type == MessageType::ConnectionRequest )
		{
			const ConnectionRequestMessage& connectionRequestMessage =
			    static_cast< const ConnectionRequestMessage& >( message );
			ProcessConnectionRequest( connectionRequestMessage, address );
		}
		else
		{
			LOG_WARNING( "Server only process Connection request messages from unknown peers. Any other type of "
			             "message will be discarded." );
		}
	}

	void Server::ProcessConnectionRequest( const ConnectionRequestMessage& message, const Address& address )
	{
		std::string ip_and_port;
		address.GetFull( ip_and_port );
		LOG_INFO( "Processing connection request from [%s] with salt number %d", ip_and_port.c_str(),
		          message.clientSalt );

		RemotePeersHandlerResult isAbleToConnectResult = _remotePeersHandler.IsRemotePeerAbleToConnect( address );

		if ( isAbleToConnectResult ==
		     RemotePeersHandlerResult::RPH_SUCCESS ) // If there is green light keep with the connection pipeline.
		{
			uint64 clientSalt = message.clientSalt;
			uint64 serverSalt = GenerateServerSalt();
			AddRemotePeer( address, _nextAssignedRemotePeerID, clientSalt, serverSalt );
			++_nextAssignedRemotePeerID;

			RemotePeer* remotePeer = _remotePeersHandler.GetRemotePeerFromAddress( address );
			CreateConnectionChallengeMessage( *remotePeer );
		}
		else if ( isAbleToConnectResult ==
		          RemotePeersHandlerResult::RPH_ALREADYEXIST ) // If the client is already connected just send a
		                                                       // connection approved message
		{
			RemotePeer* remotePeer = _remotePeersHandler.GetRemotePeerFromAddress( address );

			RemotePeerState remotePeerState = remotePeer->GeturrentState();
			if ( remotePeerState == RemotePeerState::Connected )
			{
				CreateConnectionApprovedMessage( *remotePeer );
				LOG_INFO( "The client is already connected, sending connection approved..." );
			}
			else if ( remotePeerState == RemotePeerState::Connecting )
			{
				CreateConnectionChallengeMessage( *remotePeer );
				LOG_INFO( "The client is already trying to connect, sending connection challenge..." );
			}
		}
		else if ( isAbleToConnectResult ==
		          RemotePeersHandlerResult::RPH_FULL ) // If all the client slots are full deny the connection
		{
			SendConnectionDeniedPacket( address, ConnectionFailedReasonType::CFR_SERVER_FULL );
			LOG_WARNING( "All available connection slots are full. Denying incoming connection..." );
		}
	}

	void Server::CreateDisconnectionMessage( RemotePeer& remotePeer )
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();

		std::unique_ptr< Message > message = messageFactory.LendMessage( MessageType::Disconnection );
		if ( message == nullptr )
		{
			LOG_ERROR(
			    "Can't create new Disconnection Message because the MessageFactory has returned a null message" );
			return;
		}

		std::unique_ptr< DisconnectionMessage > disconnectionMessage(
		    static_cast< DisconnectionMessage* >( message.release() ) );

		disconnectionMessage->prefix = remotePeer.GetDataPrefix();
		remotePeer.AddMessage( std::move( disconnectionMessage ) );

		LOG_INFO( "Disconnection message created." );
	}

	void Server::CreateTimeResponseMessage( RemotePeer& remotePeer, const TimeRequestMessage& timeRequest )
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr< Message > message( messageFactory.LendMessage( MessageType::TimeResponse ) );

		std::unique_ptr< TimeResponseMessage > timeResponseMessage(
		    static_cast< TimeResponseMessage* >( message.release() ) );
		timeResponseMessage->SetOrdered( true );
		timeResponseMessage->remoteTime = timeRequest.remoteTime;

		TimeClock& timeClock = TimeClock::GetInstance();
		timeResponseMessage->serverTime = timeClock.GetLocalTimeMilliseconds();

		// Find remote client
		remotePeer.AddMessage( std::move( timeResponseMessage ) );
	}

	void Server::CreateConnectionChallengeMessage( RemotePeer& remotePeer )
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();

		std::unique_ptr< Message > message = messageFactory.LendMessage( MessageType::ConnectionChallenge );
		if ( message == nullptr )
		{
			LOG_ERROR( "Can't create new Connection Challenge Message because the MessageFactory has returned a null "
			           "message" );
			return;
		}

		std::unique_ptr< ConnectionChallengeMessage > connectionChallengePacket(
		    static_cast< ConnectionChallengeMessage* >( message.release() ) );
		connectionChallengePacket->clientSalt = remotePeer.GetClientSalt();
		connectionChallengePacket->serverSalt = remotePeer.GetServerSalt();
		remotePeer.AddMessage( std::move( connectionChallengePacket ) );

		LOG_INFO( "Connection challenge message created." );
	}

	void Server::SendConnectionDeniedPacket( const Address& address, ConnectionFailedReasonType reason ) const
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();

		std::unique_ptr< Message > message = messageFactory.LendMessage( MessageType::ConnectionDenied );
		std::unique_ptr< ConnectionDeniedMessage > connectionDeniedMessage(
		    static_cast< ConnectionDeniedMessage* >( message.release() ) );
		connectionDeniedMessage->reason = reason;

		NetworkPacket packet = NetworkPacket();
		packet.AddMessage( std::move( connectionDeniedMessage ) );

		LOG_INFO( "Sending connection denied..." );
		SendPacketToAddress( packet, address );

		while ( packet.GetNumberOfMessages() > 0 )
		{
			std::unique_ptr< Message > messageToReturn = packet.GetMessages();
			messageFactory.ReleaseMessage( std::move( messageToReturn ) );
		}
	}

	void Server::ProcessConnectionChallengeResponse( const ConnectionChallengeResponseMessage& message,
	                                                 RemotePeer& remotePeer )
	{
		std::string ip_and_port;
		remotePeer.GetAddress().GetFull( ip_and_port );
		LOG_INFO( "Processing connection challenge response from [%s]", ip_and_port.c_str() );

		if ( remotePeer.GeturrentState() == RemotePeerState::Connected )
		{
			LOG_INFO( "The remote peer is already connected. Sending connection approved..." );
			CreateConnectionApprovedMessage( remotePeer );
			return;
		}

		uint64 dataPrefix = message.prefix;

		if ( remotePeer.GetDataPrefix() == dataPrefix )
		{
			ConnectRemotePeer( remotePeer );

			// Send connection approved packet
			CreateConnectionApprovedMessage( remotePeer );
			LOG_INFO( "Connection approved" );
		}
		else
		{
			LOG_INFO( "Connection denied due to not wrong data prefix" );
			SendConnectionDeniedPacket( remotePeer.GetAddress(), ConnectionFailedReasonType::CFR_UNKNOWN );

			StartDisconnectingRemotePeer( remotePeer.GetClientIndex(), false, ConnectionFailedReasonType::CFR_UNKNOWN );
		}
	}

	// TODO REFACTOR THIS METHOD
	void Server::ProcessTimeRequest( const TimeRequestMessage& message, RemotePeer& remotePeer )
	{
		LOG_INFO( "PROCESSING TIME REQUEST" );
		CreateTimeResponseMessage( remotePeer, message );
	}

	void Server::ProcessInputs( const InputStateMessage& message, RemotePeer& remotePeer )
	{
		IInputState* inputState = _inputsFactory->Create();
		assert( inputState != nullptr );

		Buffer buffer( message.data, message.dataSize );
		inputState->Deserialize( buffer );
		_remotePeerInputsHandler.AddInputState( inputState, remotePeer.GetClientIndex() );
	}

	void Server::ProcessDisconnection( const DisconnectionMessage& message, RemotePeer& remotePeer )
	{
		uint64 dataPrefix = message.prefix;
		if ( dataPrefix != remotePeer.GetDataPrefix() )
		{
			LOG_WARNING( "Packet prefix does not match. Skipping message..." );
			return;
		}

		LOG_INFO( "Disconnection message received from remote peer with reason code equal to %hhu. Disconnecting "
		          "remove peer...",
		          message.reason );

		StartDisconnectingRemotePeer( remotePeer.GetClientIndex(), false, ConnectionFailedReasonType::CFR_UNKNOWN );
	}

	void Server::CreateConnectionApprovedMessage( RemotePeer& remotePeer )
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr< Message > message = messageFactory.LendMessage( MessageType::ConnectionAccepted );
		if ( message == nullptr )
		{
			LOG_ERROR(
			    "Can't create new Connection Accepted Message because the MessageFactory has returned a null message" );
			return;
		}

		std::unique_ptr< ConnectionAcceptedMessage > connectionAcceptedPacket(
		    static_cast< ConnectionAcceptedMessage* >( message.release() ) );
		connectionAcceptedPacket->prefix = remotePeer.GetDataPrefix();
		connectionAcceptedPacket->clientIndexAssigned = remotePeer.GetClientIndex();
		remotePeer.AddMessage( std::move( connectionAcceptedPacket ) );
	}

	void Server::SendPacketToRemotePeer( const RemotePeer& remotePeer, const NetworkPacket& packet ) const
	{
		SendPacketToAddress( packet, remotePeer.GetAddress() );
	}

	void Server::TickReplication()
	{
		auto validRemotePeersIt = _remotePeersHandler.GetValidRemotePeersIterator();
		auto pastTheEndIt = _remotePeersHandler.GetValidRemotePeersPastTheEndIterator();

		for ( ; validRemotePeersIt != pastTheEndIt; ++validRemotePeersIt )
		{
			std::vector< std::unique_ptr< ReplicationMessage > > replication_messages;
			_replicationManager.Server_ReplicateWorldState( ( *validRemotePeersIt )->GetClientIndex(),
			                                                replication_messages );

			auto it = replication_messages.begin();
			for ( ; it != replication_messages.end(); ++it )
			{
				( *validRemotePeersIt )->AddMessage( std::move( *it ) );
			}
		}

		_replicationManager.ClearReplicationMessages();
	}

	void Server::RemoveReplicationEntitiesControlledByPeer( uint32 id )
	{
		_replicationManager.RemoveNetworkEntitiesControllerByPeer( id );
	}

	bool Server::StopConcrete()
	{
		return true;
	}
} // namespace NetLib
