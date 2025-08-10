#include "peer.h"

#include <memory>
#include <cassert>

#include "communication/network_packet.h"
#include "communication/message.h"
#include "communication/message_factory.h"

#include "logger.h"

#include "core/buffer.h"
#include "core/remote_peer.h"

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
			LOG_WARNING( "You are trying to call Peer::PreTick on a Peer that is disconnected" );
			return false;
		}

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

		TickRemotePeers( elapsedTime );
		TickConcrete( elapsedTime );
		FinishRemotePeersDisconnection();

		SendDataToRemotePeers();

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

	uint32 Peer::GetMetric( uint32 remote_peer_id, const std::string& metric_name, const std::string& value_type ) const
	{
		uint32 result = 0;
		const RemotePeer* remotePeer = _remotePeersHandler.GetRemotePeerFromId( remote_peer_id );
		if ( remotePeer != nullptr )
		{
			result = remotePeer->GetMetric( metric_name, value_type );
		}
		else
		{
			LOG_WARNING( "You are trying to get a metric from a remote peer that doesn't exist. ID: %u",
			             remote_peer_id );
		}

		return result;
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
	    , _remotePeersHandler( maxConnections )
	    , _onLocalPeerConnect()
	    , _onLocalPeerDisconnect()
	    , _isStopRequested( false )
	    , _stopRequestShouldNotifyRemotePeers( false )
	    , _stopRequestReason( ConnectionFailedReasonType::CFR_UNKNOWN )
	    , _currentTick( 0 )
	{
		_receiveBuffer = new uint8[ _receiveBufferSize ];
		_sendBuffer = new uint8[ _sendBufferSize ];
	}

	void Peer::SendPacketToAddress( const NetworkPacket& packet, const Address& address ) const
	{
		Buffer buffer = Buffer( _sendBuffer, packet.Size() );
		packet.Write( buffer );

		_socket.SendTo( _sendBuffer, packet.Size(), address );
	}

	bool Peer::AddRemotePeer( const Address& addressInfo, uint16 id, uint64 clientSalt, uint64 serverSalt )
	{
		bool addedSuccesfully = _remotePeersHandler.AddRemotePeer( addressInfo, id, clientSalt, serverSalt );

		return addedSuccesfully;
	}

	void Peer::ConnectRemotePeer( RemotePeer& remotePeer )
	{
		remotePeer.SetConnected();
		ExecuteOnRemotePeerConnect( remotePeer.GetClientIndex() );
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

		const uint32 id = remotePeer.GetClientIndex();
		bool removedSuccesfully = _remotePeersHandler.RemoveRemotePeer( id );
		assert( removedSuccesfully );
		ExecuteOnRemotePeerDisconnect( id );
	}

	void Peer::CreateDisconnectionPacket( const RemotePeer& remotePeer, ConnectionFailedReasonType reason )
	{
		NetworkPacket packet;
		packet.SetHeaderChannelType( TransmissionChannelType::UnreliableUnordered );

		MessageFactory& messageFactory = MessageFactory::GetInstance();
		std::unique_ptr< Message > message = messageFactory.LendMessage( MessageType::Disconnection );

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

	void Peer::ProcessReceivedData()
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
				ProcessDatagram( buffer, remoteAddress );
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

		ProcessNewRemotePeerMessages();
	}

	void Peer::ProcessDatagram( Buffer& buffer, const Address& address )
	{
		// Read incoming packet
		NetworkPacket packet = NetworkPacket();
		packet.Read( buffer );

		RemotePeer* remotePeer = _remotePeersHandler.GetRemotePeerFromAddress( address );
		bool isPacketFromRemotePeer = ( remotePeer != nullptr );
		if ( isPacketFromRemotePeer )
		{
			remotePeer->ProcessPacket( packet );
		}
		else
		{
			MessageFactory& messageFactory = MessageFactory::GetInstance();
			while ( packet.GetNumberOfMessages() > 0 )
			{
				std::unique_ptr< Message > message = packet.GetMessages();

				ProcessMessageFromUnknownPeer( *message, address );
				messageFactory.ReleaseMessage( std::move( message ) );
			}
		}
	}

	void Peer::ProcessNewRemotePeerMessages()
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

	void Peer::TickRemotePeers( float32 elapsedTime )
	{
		auto validRemotePeersIt = _remotePeersHandler.GetValidRemotePeersIterator();
		auto pastTheEndIt = _remotePeersHandler.GetValidRemotePeersPastTheEndIterator();

		for ( ; validRemotePeersIt != pastTheEndIt; ++validRemotePeersIt )
		{
			RemotePeer& remotePeer = **validRemotePeersIt;
			remotePeer.Tick( elapsedTime );

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

	void Peer::SendDataToAddress( const Buffer& buffer, const Address& address ) const
	{
		_socket.SendTo( buffer.GetData(), buffer.GetSize(), address );
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
