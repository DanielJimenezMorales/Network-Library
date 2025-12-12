#include "server.h"

#include <cassert>
#include <memory>

#include "logger.h"
#include "asserts.h"

#include "core/time_clock.h"

#include "inputs/i_input_state.h"
#include "inputs/i_input_state_factory.h"

#include "communication/message.h"
#include "communication/message_factory.h"
#include "communication/network_packet.h"
#include "communication/network_packet_utils.h"

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

	bool Server::StartServer( uint32 port )
	{
		return Start( "0.0.0.0", port );
	}

	bool Server::CreateNetworkEntity( uint32 entityType, uint32 controlledByPeerId, float32 posX, float32 posY )
	{
		if ( GetConnectionState() != PeerConnectionState::PCS_Connected )
		{
			LOG_WARNING( "Can't create Network entity of type %d because the server is not connected.",
			             static_cast< int >( entityType ) );
			return false;
		}

		_replicationManager.CreateNetworkEntity( _messageFactory, entityType, controlledByPeerId, posX, posY );
		return true;
	}

	void Server::DestroyNetworkEntity( uint32 entityId )
	{
		if ( GetConnectionState() != PeerConnectionState::PCS_Connected )
		{
			LOG_WARNING( "Can't destroy Network entity with ID: %d because the server is not connected.",
			             static_cast< int >( entityId ) );
			return;
		}

		_replicationManager.RemoveNetworkEntity( _messageFactory, entityId );
	}

	void Server::RegisterInputStateFactory( IInputStateFactory* factory )
	{
		// TODO Create a method for releasing all the inputs consumed during the current tick
		assert( factory != nullptr );
		_inputsFactory = factory;
	}

	const IInputState* Server::GetInputFromRemotePeer( uint32 remote_peer_id )
	{
		return _remotePeerInputsHandler.PopNextInputFromRemotePeer( remote_peer_id );
	}

	const IInputState* Server::GetLastInputPoppedFromRemotePeer( uint32 remote_peer_id ) const
	{
		return _remotePeerInputsHandler.GetLastInputPoppedFromRemotePeer( remote_peer_id );
	}

	bool Server::EnableInputBufferForRemotePeer( uint32 remote_peer_id )
	{
		return _remotePeerInputsHandler.EnableInputBuffer( remote_peer_id );
	}

	bool Server::DisableInputBufferForRemotePeer( uint32 remote_peer_id )
	{
		return _remotePeerInputsHandler.DisableInputBuffer( remote_peer_id );
	}

	Server::~Server()
	{
	}

	bool Server::StartConcrete( const std::string& ip, uint32 port )
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

	void Server::ProcessMessageFromPeer( const Message& message, RemotePeer& remotePeer )
	{
		MessageType messageType = message.GetHeader().type;
		LOG_INFO( "Me proceso type %d", static_cast< int >( messageType ) );

		switch ( messageType )
		{
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
			case MessageType::PingPong:
				{
					break;
				}
			default:
				LOG_WARNING( "Invalid Message type, ignoring it..." );
				break;
		}
	}

	void Server::CreateDisconnectionMessage( RemotePeer& remotePeer )
	{
		std::unique_ptr< Message > message = _messageFactory.LendMessage( MessageType::Disconnection );
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
		std::unique_ptr< Message > message( _messageFactory.LendMessage( MessageType::TimeResponse ) );

		std::unique_ptr< TimeResponseMessage > timeResponseMessage(
		    static_cast< TimeResponseMessage* >( message.release() ) );
		timeResponseMessage->SetOrdered( true );
		timeResponseMessage->remoteTime = timeRequest.remoteTime;

		TimeClock& timeClock = TimeClock::GetInstance();
		timeResponseMessage->serverTime = static_cast< uint32 >( timeClock.GetLocalTimeMilliseconds() );

		// Find remote client
		remotePeer.AddMessage( std::move( timeResponseMessage ) );
	}

	// TODO REFACTOR THIS METHOD
	void Server::ProcessTimeRequest( const TimeRequestMessage& message, RemotePeer& remotePeer )
	{
		LOG_INFO( "PROCESSING TIME REQUEST" );
		CreateTimeResponseMessage( remotePeer, message );
	}

	void Server::ProcessInputs( const InputStateMessage& message, RemotePeer& remotePeer )
	{
		const bool areInputsEnabled =
		    _remotePeerInputsHandler.GetInputsBufferAvailability( remotePeer.GetClientIndex() );
		if ( areInputsEnabled )
		{
			IInputState* inputState = _inputsFactory->Create();
			assert( inputState != nullptr );

			Buffer buffer( message.data, message.dataSize );
			if ( !inputState->Deserialize( buffer ) )
			{
				LOG_ERROR( "Server::%s, Failed to deserialize input state from remote peer %u. Ignoring input...",
				           THIS_FUNCTION_NAME, remotePeer.GetClientIndex() );
			}
			else
			{
				_remotePeerInputsHandler.AddInputState( inputState, remotePeer.GetClientIndex() );
			}
		}
		else
		{
			LOG_INFO( "Server::%s, Inputs buffer for remote peer %u are disabled. Ignoring input...",
			          THIS_FUNCTION_NAME, remotePeer.GetClientIndex() );
		}
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

	void Server::TickReplication()
	{
		auto validRemotePeersIt = _remotePeersHandler.GetValidRemotePeersIterator();
		auto pastTheEndIt = _remotePeersHandler.GetValidRemotePeersPastTheEndIterator();

		for ( ; validRemotePeersIt != pastTheEndIt; ++validRemotePeersIt )
		{
			std::vector< std::unique_ptr< ReplicationMessage > > replication_messages;
			_replicationManager.Server_ReplicateWorldState( _messageFactory, ( *validRemotePeersIt )->GetClientIndex(),
			                                                replication_messages );

			auto it = replication_messages.begin();
			for ( ; it != replication_messages.end(); ++it )
			{
				( *validRemotePeersIt )->AddMessage( std::move( *it ) );
			}
		}

		_replicationManager.ClearReplicationMessages( _messageFactory );
	}

	void Server::RemoveReplicationEntitiesControlledByPeer( uint32 id )
	{
		_replicationManager.RemoveNetworkEntitiesControllerByPeer( _messageFactory, id );
	}

	bool Server::StopConcrete()
	{
		return true;
	}

	void Server::OnPendingConnectionAccepted( const PendingConnectionData& data )
	{
		ASSERT( !data.startedLocally,
		        "Server-side can't receive a connection accepted that was started locally. Server "
		        "does not start conenctions locally." );

		_remotePeerInputsHandler.CreateInputsBuffer( data.id );
	}

	void Server::InternalOnRemotePeerDisconnect( const RemotePeer& remote_peer )
	{
		_remotePeerInputsHandler.RemoveInputsBuffer( remote_peer.GetClientIndex() );
	}
} // namespace NetLib
