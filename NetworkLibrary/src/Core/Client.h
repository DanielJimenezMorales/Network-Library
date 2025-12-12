#pragma once

// Define this before any include in order to be able to use std::numeric_limits<uint64>::min() and
// std::numeric_limits<uint64>::max() methods and not getting errors with the ones from Windows.h
#define NOMINMAX
#include <list>

#include "core/peer.h"
#include "core/address.h"

#include "time/time_syncer.h"

#include "replication/replication_messages_processor.h"

namespace NetLib
{
	class ConnectionChallengeMessage;
	class ConnectionAcceptedMessage;
	class ConnectionDeniedMessage;
	class DisconnectionMessage;
	class TimeResponseMessage;
	class ReplicationMessage;
	class IInputState;

	// TODO There's a redundancy between ClientState and PeerConnectionState. We should make the ClientState to only be
	// for the different connection states and let the Peer enum to decide the final connect disconnect
	enum ClientState
	{
		CS_Disconnected = 0,
		CS_Connected = 1
	};

	class Client : public Peer
	{
		public:
			Client( float32 serverMaxInactivityTimeout );
			Client( const Client& ) = delete;

			Client& operator=( const Client& ) = delete;

			~Client() override;

			bool StartClient( const std::string& server_ip, uint32 server_port );

			void SendInputs( const IInputState& inputState );
			uint32 GetLocalClientId() const;

			template < typename Functor >
			uint32 SubscribeToOnNetworkEntityCreate( Functor&& functor );

			template < typename Functor >
			uint32 SubscribeToOnNetworkEntityDestroy( Functor&& functor );

		protected:
			bool StartConcrete( const std::string& ip, uint32 port ) override;
			void ProcessMessageFromPeer( const Message& message, RemotePeer& remotePeer ) override;
			void TickConcrete( float32 elapsedTime ) override;
			bool StopConcrete() override;

			void OnPendingConnectionAccepted( const PendingConnectionData& data ) override;
			void OnPendingConnectionDenied( const PendingConnectionFailedData& data ) override;

			void InternalOnRemotePeerDisconnect( const RemotePeer& remote_peer ) override {};

		private:
			void ProcessDisconnection( const DisconnectionMessage& message, RemotePeer& remotePeer );
			void ProcessTimeResponse( const TimeResponseMessage& message );
			void ProcessReplicationAction( const ReplicationMessage& message );

			void OnServerDisconnect();

			Address _serverAddress;
			ClientState _currentState;
			// TODO We can probably make this a var within the Peer.h as it's shared by client and server
			uint32 _clientIndex;

			uint32 inGameMessageID; // Only for RUDP testing purposes. Delete later!

			// Time requests related
			TimeSyncer _timeSyncer;

			ReplicationMessagesProcessor _replicationMessagesProcessor;
	};

	template < typename Functor >
	inline uint32 Client::SubscribeToOnNetworkEntityCreate( Functor&& functor )
	{
		return _replicationMessagesProcessor.SubscribeToOnNetworkEntityCreate( std::forward< Functor >( functor ) );
	}

	template < typename Functor >
	inline uint32 Client::SubscribeToOnNetworkEntityDestroy( Functor&& functor )
	{
		return _replicationMessagesProcessor.SubscribeToOnNetworkEntityDestroy( std::forward< Functor >( functor ) );
	}
} // namespace NetLib
