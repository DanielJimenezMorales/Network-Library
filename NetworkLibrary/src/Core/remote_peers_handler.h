#pragma once
#include <vector>
#include <queue>
#include <unordered_set>

#include "numeric_types.h"

#include "core/remote_peer.h"

namespace NetLib
{
	class Address;
	class MessageFactory;

	enum RemotePeersHandlerResult : uint8
	{
		RPH_ERROR = 0,
		RPH_SUCCESS = 1,
		RPH_FULL = 2,
		RPH_ALREADYEXIST = 3
	};

	// If the local peer does not receive any message from a remote peer during REMOTE_PEER_INACTIVITY_TIME seconds it
	// will be considered inactive and it will be disconnected with ConnectionFailedReasonType::CFR_TIMEOUT reason
	const float32 REMOTE_PEER_INACTIVITY_TIME = 5.0f;

	class RemotePeersHandler
	{
		public:
			RemotePeersHandler();

			void Initialize( uint32 max_connections, MessageFactory* message_factory );

			void TickRemotePeers( float32 elapsedTime, MessageFactory& message_factory );

			bool AddRemotePeer( const Address& addressInfo, uint16 id, uint64 clientSalt, uint64 serverSalt );
			int32 FindFreeRemotePeerSlot() const;
			RemotePeer* GetRemotePeerFromAddress( const Address& address );
			RemotePeer* GetRemotePeerFromId( uint32 id );
			const RemotePeer* GetRemotePeerFromId( uint32 id ) const;
			bool IsRemotePeerAlreadyConnected( const Address& address ) const;
			bool DoesRemotePeerIdExist( uint32 id ) const;
			RemotePeersHandlerResult IsRemotePeerAbleToConnect( const Address& address ) const;

			std::unordered_set< RemotePeer* >::iterator GetValidRemotePeersIterator();
			std::unordered_set< RemotePeer* >::iterator GetValidRemotePeersPastTheEndIterator();

			void RemoveAllRemotePeers();
			bool RemoveRemotePeer( uint32 remotePeerId );

		private:
			int32 GetIndexFromId( uint32 id ) const;

			uint32 _maxConnections;
			std::vector< bool > _remotePeerSlots;
			std::vector< RemotePeer > _remotePeers;
			std::unordered_set< RemotePeer* > _validRemotePeers;
			bool _isInitialized;
	};
} // namespace NetLib
