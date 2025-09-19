#pragma once
#include "numeric_types.h"

#include <vector>
#include <list>
#include <string>

#include "delegate.hpp"

#include "core/address.h"
#include "core/socket.h"
#include "core/remote_peers_handler.h"

#include "transmission_channels/transmission_channel.h"

class Buffer;

namespace NetLib
{
	class Message;
	class NetworkPacket;
	class RemotePeer;
	class Buffer;

	enum ConnectionFailedReasonType : uint8
	{
		CFR_UNKNOWN = 0,           // Unexpect
		CFR_TIMEOUT = 1,           // The peer is inactive
		CFR_SERVER_FULL = 2,       // The server can't handle more connections, it has reached its maximum
		CFR_PEER_SHUT_DOWN = 3,    // The peer has shut down its Network system
		CFR_CONNECTION_TIMEOUT = 4 // The in process connection has taken too long
	};

	struct RemotePeerDisconnectionData
	{
			uint32 id;
			bool shouldNotify;
			ConnectionFailedReasonType reason;
	};

	enum class PeerType : uint8
	{
		NONE = 0,
		CLIENT = 1,
		SERVER = 2
	};

	enum PeerConnectionState : uint8
	{
		PCS_Disconnected = 0,
		PCS_Connecting = 1,
		PCS_Connected = 2
	};

	// TODO Set ordered and reliable flags in all the connection messages such as challenge response, connection
	// approved...
	class Peer
	{
		public:
			/// <summary>
			/// Connects the Peer to the specified IP and port. For a server, the user will need to pass an ip of
			/// "0.0.0.0" to listen for all connections. The server's connection pipeline will happen inmediately. For a
			/// client, the user will need to specify a valid IP where the server is at and the port will be 0 so the
			/// system picks a random one. The client's connection pipeline will not be inmediate as it needs to get
			/// approval from server
			/// </summary>
			/// <param name="ip">The IP to connect at. For client = server IP. For server = "0.0.0.0"</param>
			/// <param name="port">The port to listen at. For client = 0. For server = a non-zero port</param>
			/// <returns></returns>
			bool Start( const std::string& ip, uint32 port );
			bool PreTick();
			bool Tick( float32 elapsedTime );
			bool Stop();

			PeerConnectionState GetConnectionState() const { return _connectionState; }
			RemotePeerState GetRemotePeerState( uint32 remote_peer_id ) const;

			/// <summary>
			/// Returns the type of this peer. It can be either a client or a server.
			/// </summary>
			PeerType GetPeerType() const { return _type; }
			uint32 GetCurrentTick() const { return _currentTick; }

			/// <summary>
			/// Get the metric value from a remote peer. If the remote peer doesn't exist or the metric is not found, a
			/// value of 0 is returned.
			/// </summary>
			/// <param name="remote_peer_id">The remote peer id</param>
			/// <param name="metric_name">The name of the metric. See metrics/metric_names.h for more info</param>
			/// <param name="value_type">The type of value you want to get. See metrics/metric_names.h for more
			/// info</param>
			/// <returns>The metric value on success or 0 on failure</returns>
			uint32 GetMetric( uint32 remote_peer_id, const std::string& metric_name,
			                  const std::string& value_type ) const;

			float64 GetLocalTime() const;
			float64 GetServerTime() const;

			// Delegates related
			template < typename Functor >
			Common::Delegate<>::SubscriptionHandler SubscribeToOnLocalPeerConnect( Functor&& functor );
			bool UnsubscribeToOnPeerConnected( const Common::Delegate<>::SubscriptionHandler& handler );

			template < typename Functor >
			Common::Delegate< ConnectionFailedReasonType >::SubscriptionHandler SubscribeToOnLocalPeerDisconnect(
			    Functor&& functor );
			bool UnsubscribeToOnPeerDisconnected(
			    const Common::Delegate< ConnectionFailedReasonType >::SubscriptionHandler& handler );

			template < typename Functor >
			Common::Delegate< uint32 >::SubscriptionHandler SubscribeToOnRemotePeerDisconnect( Functor&& functor );
			bool UnsubscribeToOnRemotePeerDisconnect( const Common::Delegate< uint32 >::SubscriptionHandler& handler );

			template < typename Functor >
			Common::Delegate< uint32 >::SubscriptionHandler SubscribeToOnRemotePeerConnect( Functor&& functor );
			bool UnsubscribeToOnRemotePeerConnect( const Common::Delegate< uint32 >::SubscriptionHandler& handler );

			virtual ~Peer();

		protected:
			Peer( PeerType type, uint32 maxConnections, uint32 receiveBufferSize, uint32 sendBufferSize );
			Peer( const Peer& ) = delete;

			Peer& operator=( const Peer& ) = delete;

			virtual bool StartConcrete( const std::string& ip, uint32 port ) = 0;
			virtual void ProcessMessageFromPeer( const Message& message, RemotePeer& remotePeer ) = 0;
			virtual void ProcessMessageFromUnknownPeer( const Message& message, const Address& address ) = 0;
			virtual void TickConcrete( float32 elapsedTime ) = 0;
			virtual bool StopConcrete() = 0;

			void SendPacketToAddress( const NetworkPacket& packet, const Address& address ) const;
			bool AddRemotePeer( const Address& addressInfo, uint16 id, uint64 clientSalt, uint64 serverSalt );
			void ConnectRemotePeer( RemotePeer& remotePeer );
			bool BindSocket( const Address& address ) const;

			void StartDisconnectingRemotePeer( uint32 id, bool shouldNotify, ConnectionFailedReasonType reason );

			void RequestStop( bool shouldNotifyRemotePeers, ConnectionFailedReasonType reason );

			// Delegates related

			/// <summary>
			/// Called during OnRemotePeerConnect. This function is used for events happening inside the network library
			/// code. This function will be called before teh OnRemotePeerConnect Delegate
			/// </summary>
			virtual void InternalOnRemotePeerConnect( RemotePeer& remote_peer ) = 0;
			virtual void InternalOnRemotePeerDisconnect( const RemotePeer& remote_peer ) = 0;
			void ExecuteOnLocalPeerConnect();
			void ExecuteOnLocalPeerDisconnect( ConnectionFailedReasonType reason );

			RemotePeersHandler _remotePeersHandler;

		private:
			void ProcessReceivedData();
			void ProcessDatagram( Buffer& buffer, const Address& address );
			void ProcessNewRemotePeerMessages();

			void SetConnectionState( PeerConnectionState state );

			// Remote peer related
			void TickRemotePeers( float32 elapsedTime );
			void DisconnectAllRemotePeers( bool shouldNotify, ConnectionFailedReasonType reason );
			void DisconnectRemotePeer( const RemotePeer& remotePeer, bool shouldNotify,
			                           ConnectionFailedReasonType reason );

			void CreateDisconnectionPacket( const RemotePeer& remotePeer, ConnectionFailedReasonType reason );

			/// <summary>
			/// Sends pending data to all the connected remote peers
			/// </summary>
			void SendDataToRemotePeers();

			void SendDataToAddress( const Buffer& buffer, const Address& address ) const;

			bool DoesRemotePeerIdExistInPendingDisconnections( uint32 id ) const;
			void FinishRemotePeersDisconnection();

			void StopInternal();

			// Delegates related
			void ExecuteOnRemotePeerConnect( uint32 remotePeerId );
			void ExecuteOnRemotePeerDisconnect( uint32 id );

			PeerType _type;
			PeerConnectionState _connectionState;
			Address _address;
			Socket _socket;

			const uint32 _receiveBufferSize;
			uint8* _receiveBuffer;
			const uint32 _sendBufferSize;
			uint8* _sendBuffer;

			uint32 _currentTick;

			// Stop request
			bool _isStopRequested;
			bool _stopRequestShouldNotifyRemotePeers;
			ConnectionFailedReasonType _stopRequestReason;

			std::list< RemotePeerDisconnectionData > _remotePeerPendingDisconnections;

			Common::Delegate<> _onLocalPeerConnect;
			Common::Delegate< ConnectionFailedReasonType > _onLocalPeerDisconnect;
			Common::Delegate< uint32 > _onRemotePeerConnect;
			Common::Delegate< uint32 > _onRemotePeerDisconnect;
	};

	template < typename Functor >
	inline Common::Delegate<>::SubscriptionHandler Peer::SubscribeToOnLocalPeerConnect( Functor&& functor )
	{
		return _onLocalPeerConnect.AddSubscriber( std::forward< Functor >( functor ) );
	}

	template < typename Functor >
	inline Common::Delegate< ConnectionFailedReasonType >::SubscriptionHandler Peer::SubscribeToOnLocalPeerDisconnect(
	    Functor&& functor )
	{
		return _onLocalPeerDisconnect.AddSubscriber( std::forward< Functor >( functor ) );
	}

	template < typename Functor >
	inline Common::Delegate< uint32 >::SubscriptionHandler Peer::SubscribeToOnRemotePeerDisconnect( Functor&& functor )
	{
		return _onRemotePeerDisconnect.AddSubscriber( std::forward< Functor >( functor ) );
	}
	template < typename Functor >
	inline Common::Delegate< uint32 >::SubscriptionHandler Peer::SubscribeToOnRemotePeerConnect( Functor&& functor )
	{
		return _onRemotePeerConnect.AddSubscriber( std::forward< Functor >( functor ) );
	}
} // namespace NetLib
