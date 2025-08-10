#pragma once
#include <cassert>
#include <vector>

#include "core/peer.h"
#include "core/client.h"
#include "core/server.h"

#include <queue>

class IInputStateFactory;

struct NetworkPeerGlobalComponent
{
		NetworkPeerGlobalComponent()
		    : peer( nullptr )
		    , inputStateFactory( nullptr )
		    , unprocessedConnectedRemotePeers()
		    , isTrackingRemotePeerConnect( false )
		    , remotePeerConnectSubscriptionHandler( Common::Delegate< uint32 >::SubscriptionHandler::Invalid() )
		{
		}

		~NetworkPeerGlobalComponent();

		// TODO free this memory correctly
		NetLib::Peer* peer;
		NetLib::IInputStateFactory* inputStateFactory;

		std::queue< uint32 > unprocessedConnectedRemotePeers;
		bool isTrackingRemotePeerConnect;
		Common::Delegate< uint32 >::SubscriptionHandler remotePeerConnectSubscriptionHandler;

		void TrackOnRemotePeerConnect()
		{
			auto callback = [ & ]( uint32 remotePeerId )
			{
				this->AddUnprocessedConnectedRemotePeer( remotePeerId );
			};
			remotePeerConnectSubscriptionHandler = peer->SubscribeToOnRemotePeerConnect( callback );
			isTrackingRemotePeerConnect = true;
		}

		void AddUnprocessedConnectedRemotePeer( uint32 remotePeerId )
		{
			unprocessedConnectedRemotePeers.push( remotePeerId );
		};

		NetLib::Client* GetPeerAsClient() const
		{
			assert( peer != nullptr );
			assert( peer->GetPeerType() == NetLib::PeerType::CLIENT );
			return static_cast< NetLib::Client* >( peer );
		}
		NetLib::Server* GetPeerAsServer() const
		{
			assert( peer != nullptr );
			assert( peer->GetPeerType() == NetLib::PeerType::SERVER );
			return static_cast< NetLib::Server* >( peer );
		}
};
