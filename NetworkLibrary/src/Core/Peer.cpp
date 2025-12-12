#include "peer.h"

#include <memory>
#include <cassert>

#include "communication/network_packet.h"
#include "communication/message.h"
#include "communication/message_factory.h"
#include "communication/network_packet_utils.h"

#include "connection/client_connection_pipeline.h"
#include "connection/server_connection_pipeline.h"

#include "logger.h"
#include "asserts.h"

#include "core/buffer.h"
#include "core/remote_peer.h"
#include "core/time_clock.h"

namespace NetLib
{
	bool Peer::Start( const std::string& ip, uint32 port )
	{
		if ( _connectionState != PeerConnectionState::PCS_Disconnected )
		{
			LOG_WARNING( "You are trying to call Peer::Start on a Peer that has already started" );
			return true;
		}

		SetConnectionState( PeerConnectionState::PCS_Connecting );

		if ( _socket.Start() != SocketResult::SOKT_SUCCESS )
		{
			LOG_ERROR( "Error while starting peer, aborting operation..." );
			SetConnectionState( PeerConnectionState::PCS_Disconnected );
			return false;
		}

		// TODO, This is hardcoded
		ConnectionConfiguration connectionConfiguration;
		connectionConfiguration.canStartConnections = ( _type == PeerType::CLIENT );
		connectionConfiguration.maxPendingConnections = ( _type == PeerType::CLIENT ) ? 1 : 10;
		if ( _type == PeerType::CLIENT )
		{
			connectionConfiguration.connectionPipeline = new ClientConnectionPipeline();
		}
		else if ( _type == PeerType::SERVER )
		{
			connectionConfiguration.connectionPipeline = new ServerConnectionPipeline();
		}

		if ( !_connectionManager.StartUp( connectionConfiguration, &_messageFactory ) )
		{
			LOG_ERROR( "Error while starting peer connection manager, aborting operation..." );
			SetConnectionState( PeerConnectionState::PCS_Disconnected );
			return false;
		}

		if ( !StartConcrete( ip, port ) )
		{
			LOG_ERROR( "Error while starting peer, aborting operation..." );
			SetConnectionState( PeerConnectionState::PCS_Disconnected );
			return false;
		}

		_currentTick = 1;
		LOG_INFO( "Peer started succesfully" );
		return true;
	}

	bool Peer::PreTick()
	{
		if ( _connectionState == PeerConnectionState::PCS_Disconnected )
		{
			LOG_WARNING( "You are trying to call PreTick on a Peer that is disconnected" );
			return false;
		}

		ReadReceivedData();
		ProcessReceivedData();

		return true;
	}

	bool Peer::Tick( float32 elapsedTime )
	{
		if ( _connectionState == PeerConnectionState::PCS_Disconnected )
		{
			LOG_WARNING( "You are trying to call Peer::Tick on a Peer that is disconnected" );
			return false;
		}

		TickPendingConnections( elapsedTime );
		TickRemotePeers( elapsedTime );
		TickConcrete( elapsedTime );
		FinishRemotePeersDisconnection();

		SendDataToPendingConnections();
		SendDataToRemotePeers();
		ConvertSuccessfulConnectionsInRemotePeers();
		ProcessDeniedConnections();

		if ( _isStopRequested )
		{
			StopInternal();
		}

		assert( _currentTick < MAX_UINT32 );
		++_currentTick;

		return true;
	}

	bool Peer::Stop()
	{
		RequestStop( true, ConnectionFailedReasonType::CFR_PEER_SHUT_DOWN );
		StopInternal();

		return true;
	}

	RemotePeerState Peer::GetRemotePeerState( uint32 remote_peer_id ) const
	{
		RemotePeerState result = RemotePeerState::Disconnected;
		const RemotePeer* remotePeer = _remotePeersHandler.GetRemotePeerFromId( remote_peer_id );
		if ( remotePeer != nullptr )
		{
			result = remotePeer->GeturrentState();
		}

		return result;
	}

	uint32 Peer::GetMetric( uint32 remote_peer_id, Metrics::MetricType metric_type,
	                        Metrics::ValueType value_type ) const
	{
		uint32 result = 0;
		const RemotePeer* remotePeer = _remotePeersHandler.GetRemotePeerFromId( remote_peer_id );
		if ( remotePeer != nullptr )
		{
			result = remotePeer->GetMetric( metric_type, value_type );
		}
		else
		{
			LOG_WARNING( "You are trying to get a metric from a remote peer that doesn't exist. ID: %u",
			             remote_peer_id );
		}

		return result;
	}

	float64 Peer::GetLocalTime() const
	{
		// TODO Make the time clock class not be a singleton and be a peer class variable. You will need probably a
		// system to update it within UPDATE STAGE
		return TimeClock::GetInstance().GetLocalTimeSeconds();
	}

	float64 Peer::GetServerTime() const
	{
		// TODO Make the time clock class not be a singleton and be a peer class variable. You will need probably a
		// system to update it within UPDATE STAGE
		return TimeClock::GetInstance().GetServerTimeSeconds();
	}

	bool Peer::UnsubscribeToOnRemotePeerDisconnect( const Common::Delegate< uint32 >::SubscriptionHandler& handler )
	{
		return _onRemotePeerDisconnect.DeleteSubscriber( handler );
	}

	bool Peer::UnsubscribeToOnRemotePeerConnect( const Common::Delegate< uint32 >::SubscriptionHandler& handler )
	{
		return _onRemotePeerConnect.DeleteSubscriber( handler );
	}

	Peer::~Peer()
	{
		delete[] _receiveBuffer;
		delete[] _sendBuffer;
	}

	Peer::Peer( PeerType type, uint32 maxConnections, uint32 receiveBufferSize, uint32 sendBufferSize )
	    : _type( type )
	    , _connectionState( PeerConnectionState::PCS_Disconnected )
	    , _socket()
	    , _address( Address::GetInvalid() )
	    , _receiveBufferSize( receiveBufferSize )
	    , _sendBufferSize( sendBufferSize )
	    , _remotePeersHandler()
	    , _onLocalPeerConnect()
	    , _onLocalPeerDisconnect()
	    , _isStopRequested( false )
	    , _stopRequestShouldNotifyRemotePeers( false )
	    , _stopRequestReason( ConnectionFailedReasonType::CFR_UNKNOWN )
	    , _currentTick( 0 )
	    , _messageFactory( 3 )
	    , _connectionManager()
	{
		_receiveBuffer = new uint8[ _receiveBufferSize ];
		_sendBuffer = new uint8[ _sendBufferSize ];
		_remotePeersHandler.Initialize( maxConnections, &_messageFactory );
	}

	void Peer::SendPacketToAddress( const NetworkPacket& packet, const Address& address ) const
	{
		Buffer buffer = Buffer( _sendBuffer, packet.Size() );
		packet.Write( buffer );

		_socket.SendTo( _sendBuffer, packet.Size(), address );
	}

	bool Peer::AddRemotePeer( const Address& addressInfo, uint16 id, uint64 clientSalt, uint64 serverSalt )
	{
		return _remotePeersHandler.AddRemotePeer( addressInfo, id, clientSalt, serverSalt );
	}

	bool Peer::BindSocket( const Address& address ) const
	{
		SocketResult result = _socket.Bind( address );
		if ( result != SocketResult::SOKT_SUCCESS )
		{
			return false;
		}

		return true;
	}

	void Peer::DisconnectAllRemotePeers( bool shouldNotify, ConnectionFailedReasonType reason )
	{
		if ( shouldNotify )
		{
			auto validRemotePeersIt = _remotePeersHandler.GetValidRemotePeersIterator();
			auto pastTheEndIt = _remotePeersHandler.GetValidRemotePeersPastTheEndIterator();

			for ( ; validRemotePeersIt != pastTheEndIt; ++validRemotePeersIt )
			{
				const RemotePeer& remotePeer = **validRemotePeersIt;
				CreateDisconnectionPacket( remotePeer, reason );
			}
		}

		_remotePeersHandler.RemoveAllRemotePeers();
	}

	void Peer::DisconnectRemotePeer( const RemotePeer& remotePeer, bool shouldNotify,
	                                 ConnectionFailedReasonType reason )
	{
		if ( shouldNotify )
		{
			CreateDisconnectionPacket( remotePeer, reason );
		}

		InternalOnRemotePeerDisconnect( remotePeer );
		const uint32 id = remotePeer.GetClientIndex();
		bool removedSuccesfully = _remotePeersHandler.RemoveRemotePeer( id );
		assert( removedSuccesfully );
		ExecuteOnRemotePeerDisconnect( id );
	}

	void Peer::CreateDisconnectionPacket( const RemotePeer& remotePeer, ConnectionFailedReasonType reason )
	{
		NetworkPacket packet;
		packet.SetHeaderChannelType( TransmissionChannelType::UnreliableUnordered );

		std::unique_ptr< Message > message = _messageFactory.LendMessage( MessageType::Disconnection );

		std::unique_ptr< DisconnectionMessage > disconenctionMessage(
		    static_cast< DisconnectionMessage* >( message.release() ) );
		disconenctionMessage->SetOrdered( false );
		disconenctionMessage->SetReliability( false );
		disconenctionMessage->prefix = remotePeer.GetDataPrefix();
		disconenctionMessage->reason = reason;

		packet.AddMessage( std::move( disconenctionMessage ) );
		SendPacketToAddress( packet, remotePeer.GetAddress() );
	}

	void Peer::ExecuteOnLocalPeerConnect()
	{
		SetConnectionState( PeerConnectionState::PCS_Connected );
		_onLocalPeerConnect.Execute();
	}

	void Peer::ExecuteOnLocalPeerDisconnect( ConnectionFailedReasonType reason )
	{
		_onLocalPeerDisconnect.Execute( reason );
	}

	bool Peer::UnsubscribeToOnPeerConnected( const Common::Delegate<>::SubscriptionHandler& handler )
	{
		return _onLocalPeerConnect.DeleteSubscriber( handler );
	}

	bool Peer::UnsubscribeToOnPeerDisconnected(
	    const Common::Delegate< ConnectionFailedReasonType >::SubscriptionHandler& handler )
	{
		return _onLocalPeerDisconnect.DeleteSubscriber( handler );
	}

	void Peer::ReadReceivedData()
	{
		Address remoteAddress = Address::GetInvalid();
		uint32 numberOfBytesRead = 0;
		bool arePendingDatagramsToRead = true;

		do
		{
			SocketResult result =
			    _socket.ReceiveFrom( _receiveBuffer, _receiveBufferSize, remoteAddress, numberOfBytesRead );

			if ( result == SocketResult::SOKT_SUCCESS )
			{
				// Data read succesfully. Keep going!
				Buffer buffer = Buffer( _receiveBuffer, numberOfBytesRead );
				ReadDatagram( buffer, remoteAddress );
			}
			else if ( result == SocketResult::SOKT_ERR || result == SocketResult::SOKT_WOULDBLOCK )
			{
				// An unexpected error occurred or there is no more data to read atm
				arePendingDatagramsToRead = false;
			}
			else if ( result == SocketResult::SOKT_CONNRESET )
			{
				// The remote socket got closed unexpectedly
				RemotePeer* remotePeer = _remotePeersHandler.GetRemotePeerFromAddress( remoteAddress );
				if ( remotePeer != nullptr )
				{
					StartDisconnectingRemotePeer( remotePeer->GetClientIndex(), false,
					                              ConnectionFailedReasonType::CFR_UNKNOWN );
				}
			}
		} while ( arePendingDatagramsToRead );
	}

	void Peer::ReadDatagram( Buffer& buffer, const Address& address )
	{
		// TODO Add validation for tampered or corrupted packets so it doesn't crash when a tampered message arrives.
		//  Read incoming packet
		// Read Network packet
		NetworkPacket packet;
		const bool readSuccessfully = NetworkPacketUtils::ReadNetworkPacket( buffer, _messageFactory, packet );
		if ( !readSuccessfully )
		{
			std::string ip_and_port;
			address.GetFull( ip_and_port );
			LOG_WARNING( "Received corrupted or invalid packet from %s. Discarding packet.", ip_and_port.c_str() );
			return;
		}

		// Store messages within transmission channels for being processed
		StoreReceivedMessages( packet, address );
	}

	void Peer::StoreReceivedMessages( NetworkPacket& packet, const Address& address )
	{
		RemotePeer* remotePeer = _remotePeersHandler.GetRemotePeerFromAddress( address );
		bool isPacketFromRemotePeer = ( remotePeer != nullptr );
		if ( isPacketFromRemotePeer )
		{
			remotePeer->ProcessPacket( packet );
		}
		else
		{
			_connectionManager.ProcessPacket( address, packet );
		}
	}

	void Peer::ProcessReceivedData()
	{
		auto validRemotePeersIt = _remotePeersHandler.GetValidRemotePeersIterator();
		auto pastTheEndIt = _remotePeersHandler.GetValidRemotePeersPastTheEndIterator();

		for ( ; validRemotePeersIt != pastTheEndIt; ++validRemotePeersIt )
		{
			// Process ready to process messages from remote peer
			RemotePeer& remotePeer = **validRemotePeersIt;

			while ( remotePeer.ArePendingReadyToProcessMessages() )
			{
				const Message* message = remotePeer.GetPendingReadyToProcessMessage();
				ProcessMessageFromPeer( *message, remotePeer );
			}

			remotePeer.FreeProcessedMessages();
		}
	}

	void Peer::TickPendingConnections( float32 elapsed_time )
	{
		_connectionManager.Tick( elapsed_time );
	}

	void Peer::ConvertSuccessfulConnectionsInRemotePeers()
	{
		std::vector< PendingConnectionData > successfulConnections;
		_connectionManager.GetConnectedPendingConnectionsData( successfulConnections );

		for ( auto& cit = successfulConnections.cbegin(); cit != successfulConnections.cend(); ++cit )
		{
			// TODO Change this and get rid of both salts in remote peer. Just keep the data prefix
			if ( AddRemotePeer( cit->address, cit->id, cit->dataPrefix, 0 ) )
			{
				RemotePeer* remotePeer = _remotePeersHandler.GetRemotePeerFromId( cit->id );
				ASSERT( remotePeer != nullptr, "Remote peer cannot be nullptr after its creation" );
				OnPendingConnectionAccepted( *cit );
				ExecuteOnRemotePeerConnect( cit->id );
			}
			else
			{
				LOG_ERROR( "%s Error while adding new remote peer after successful connection", THIS_FUNCTION_NAME );
			}
		}

		_connectionManager.ClearConnectedPendingConnections();
	}

	void Peer::ProcessDeniedConnections()
	{
		std::vector< PendingConnectionFailedData > deniedConnections;
		_connectionManager.GetDeniedPendingConnectionsData( deniedConnections );

		for ( auto& cit = deniedConnections.cbegin(); cit != deniedConnections.cend(); ++cit )
		{
			OnPendingConnectionDenied( *cit );
		}

		_connectionManager.ClearDeniedPendingConnections();
	}

	void Peer::TickRemotePeers( float32 elapsedTime )
	{
		auto validRemotePeersIt = _remotePeersHandler.GetValidRemotePeersIterator();
		auto pastTheEndIt = _remotePeersHandler.GetValidRemotePeersPastTheEndIterator();

		for ( ; validRemotePeersIt != pastTheEndIt; ++validRemotePeersIt )
		{
			RemotePeer& remotePeer = **validRemotePeersIt;
			remotePeer.Tick( elapsedTime, _messageFactory );

			// Start the disconnection process for those ones who are inactive
			if ( remotePeer.IsInactive() )
			{
				StartDisconnectingRemotePeer( remotePeer.GetClientIndex(), true,
				                              ConnectionFailedReasonType::CFR_TIMEOUT );
			}
		}
	}

	void Peer::SendDataToRemotePeers()
	{
		auto validRemotePeersIt = _remotePeersHandler.GetValidRemotePeersIterator();
		auto pastTheEndIt = _remotePeersHandler.GetValidRemotePeersPastTheEndIterator();

		for ( ; validRemotePeersIt != pastTheEndIt; ++validRemotePeersIt )
		{
			( *validRemotePeersIt )->SendData( _socket );
		}
	}

	void Peer::SendDataToPendingConnections()
	{
		_connectionManager.SendDataToPendingConnections( _socket );
	}

	void Peer::StartDisconnectingRemotePeer( uint32 id, bool shouldNotify, ConnectionFailedReasonType reason )
	{
		RemotePeer* remotePeer = _remotePeersHandler.GetRemotePeerFromId( id );

		if ( remotePeer != nullptr )
		{
			if ( !DoesRemotePeerIdExistInPendingDisconnections( id ) )
			{
				LOG_INFO( "EMPIEZO A DESCONECTARR" );
				RemotePeerDisconnectionData disconnectionData;
				disconnectionData.id = id;
				disconnectionData.shouldNotify = shouldNotify;
				disconnectionData.reason = reason;

				_remotePeerPendingDisconnections.push_back( disconnectionData );
			}
		}
	}

	bool Peer::DoesRemotePeerIdExistInPendingDisconnections( uint32 id ) const
	{
		bool doesIdAlreadyExist = false;
		auto cit = _remotePeerPendingDisconnections.cbegin();
		while ( cit != _remotePeerPendingDisconnections.cend() )
		{
			const RemotePeerDisconnectionData& data = *cit;
			if ( data.id == id )
			{
				doesIdAlreadyExist = true;
				break;
			}

			++cit;
		}

		return doesIdAlreadyExist;
	}

	void Peer::FinishRemotePeersDisconnection()
	{
		while ( !_remotePeerPendingDisconnections.empty() )
		{
			RemotePeerDisconnectionData& disconnectionData = _remotePeerPendingDisconnections.front();

			RemotePeer* remotePeer = _remotePeersHandler.GetRemotePeerFromId( disconnectionData.id );
			if ( remotePeer != nullptr )
			{
				DisconnectRemotePeer( *remotePeer, disconnectionData.shouldNotify, disconnectionData.reason );
			}

			_remotePeerPendingDisconnections.erase( _remotePeerPendingDisconnections.begin() );
		}
	}

	void Peer::RequestStop( bool shouldNotifyRemotePeers, ConnectionFailedReasonType reason )
	{
		_isStopRequested = true;
		_stopRequestShouldNotifyRemotePeers = shouldNotifyRemotePeers;
		_stopRequestReason = reason;
	}

	void Peer::SetConnectionState( PeerConnectionState state )
	{
		_connectionState = state;
	}

	void Peer::ExecuteOnRemotePeerDisconnect( uint32 id )
	{
		_onRemotePeerDisconnect.Execute( id );
	}

	void Peer::ExecuteOnRemotePeerConnect( uint32 remotePeerId )
	{
		_onRemotePeerConnect.Execute( remotePeerId );
	}

	void Peer::StopInternal()
	{
		if ( _connectionState == PeerConnectionState::PCS_Disconnected )
		{
			LOG_WARNING( "You are trying to call Peer::Stop on a Peer that is disconnected" );
			return;
		}

		StopConcrete();
		DisconnectAllRemotePeers( _stopRequestShouldNotifyRemotePeers, _stopRequestReason );
		_socket.Close();

		_isStopRequested = false;

		PeerConnectionState previousConnectionState = _connectionState;
		SetConnectionState( PeerConnectionState::PCS_Disconnected );
		LOG_INFO( "Peer stopped succesfully" );

		ExecuteOnLocalPeerDisconnect( _stopRequestReason );
	}
} // namespace NetLib
