#pragma once
#include <vector>

#include "core/Peer.h"

#include "inputs/remote_peer_inputs_handler.h"

#include "replication/replication_manager.h"

namespace NetLib
{
	class PendingConnection;
	class ConnectionRequestMessage;
	class ConnectionChallengeResponseMessage;
	class TimeRequestMessage;
	class InputStateMessage;
	class DisconnectionMessage;
	class IInputState;
	class IInputStateFactory;

	class Server : public Peer
	{
		public:
			Server( int32 maxConnections );
			Server( const Server& ) = delete;

			Server& operator=( const Server& ) = delete;

			bool StartServer( uint32 port );

			uint32 CreateNetworkEntity( uint32 entityType, uint32 controlledByPeerId, float32 posX, float32 posY );
			void DestroyNetworkEntity( uint32 entityId );
			// TODO Create a method for destroying all network entities controlled by a remote peer
			void RegisterInputStateFactory( IInputStateFactory* factory );
			const IInputState* GetInputFromRemotePeer( uint32 remotePeerId );

			template < typename Functor >
			uint32 SubscribeToOnNetworkEntityCreate( Functor&& functor );

			template < typename Functor >
			uint32 SubscribeToOnNetworkEntityDestroy( Functor&& functor );

			~Server() override;

		protected:
			bool StartConcrete( const std::string& ip, uint32 port ) override;
			void ProcessMessageFromPeer( const Message& message, RemotePeer& remotePeer ) override;
			void ProcessMessageFromUnknownPeer( const Message& message, const Address& address ) override;
			void TickConcrete( float32 elapsedTime ) override;
			bool StopConcrete() override;

		private:
			uint64 GenerateServerSalt() const;

			void ProcessConnectionRequest( const ConnectionRequestMessage& message, const Address& address );
			void ProcessConnectionChallengeResponse( const ConnectionChallengeResponseMessage& message,
			                                         RemotePeer& remotePeer );
			void ProcessTimeRequest( const TimeRequestMessage& message, RemotePeer& remotePeer );
			void ProcessInputs( const InputStateMessage& message, RemotePeer& remotePeer );
			void ProcessDisconnection( const DisconnectionMessage& message, RemotePeer& remotePeer );

			/// <summary>
			/// This method checks if a new client is able to connect to server
			/// </summary>
			/// <param name="address">The network address information of the client trying to connect</param>
			/// <returns>
			/// 0 = Is able to connect.
			/// 1 = Is already connected.
			/// -1 = Unable to connect, the server has reached its maximum connections.
			/// </returns>
			// int32 IsRemotePeerAbleToConnect(const Address& address) const;

			void CreateConnectionChallengeMessage( RemotePeer& remotePeer );
			void CreateConnectionApprovedMessage( RemotePeer& remotePeer );
			void CreateDisconnectionMessage( RemotePeer& remotePeer );
			void CreateTimeResponseMessage( RemotePeer& remotePeer, const TimeRequestMessage& timeRequest );
			void SendConnectionDeniedPacket( const Address& address, ConnectionFailedReasonType reason ) const;
			void SendPacketToRemotePeer( const RemotePeer& remotePeer, const NetworkPacket& packet ) const;

			void TickReplication();

			void RemoveReplicationEntitiesControlledByPeer( uint32 id );

			uint32 _nextAssignedRemotePeerID = 1;

			RemotePeerInputsHandler _remotePeerInputsHandler;
			IInputStateFactory* _inputsFactory;

			ReplicationManager _replicationManager;
	};

	template < typename Functor >
	inline uint32 Server::SubscribeToOnNetworkEntityCreate( Functor&& functor )
	{
		return _replicationManager.SubscribeToOnNetworkEntityCreate( std::forward< Functor >( functor ) );
	}

	template < typename Functor >
	inline uint32 Server::SubscribeToOnNetworkEntityDestroy( Functor&& functor )
	{
		return _replicationManager.SubscribeToOnNetworkEntityDestroy( std::forward< Functor >( functor ) );
	}
} // namespace NetLib
