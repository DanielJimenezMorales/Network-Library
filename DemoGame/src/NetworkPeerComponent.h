#pragma once
#include <cassert>
#include <vector>
#include "Peer.h"
#include "Client.h"
#include "Server.h"
#include <queue>

class INetworkEntityFactory;
class IInputStateFactory;

struct NetworkPeerComponent
{
	NetworkPeerComponent() : peer(nullptr), networkEntityFactories(), inputStateFactory(nullptr), unprocessedConnectedRemotePeers(), isTrackingRemotePeerConnect(false), remotePeerConnectSubscriptionId(0)
	{
	}

	~NetworkPeerComponent() {
		if (isTrackingRemotePeerConnect)
		{
			peer->UnsubscribeToOnPeerConnected(remotePeerConnectSubscriptionId);
		}

		delete peer;
		peer = nullptr;
		auto it = networkEntityFactories.begin();
		for (; it != networkEntityFactories.end(); ++it)
		{
			delete *it;
			*it = nullptr;
		}

		if (inputStateFactory != nullptr)
		{
			delete inputStateFactory;
			inputStateFactory = nullptr;
		}

		networkEntityFactories.clear();
	}

	//TODO free this memory correctly
	NetLib::Peer* peer;
	std::vector<INetworkEntityFactory*> networkEntityFactories;
	NetLib::IInputStateFactory* inputStateFactory;

	std::queue<uint32_t> unprocessedConnectedRemotePeers;
	bool isTrackingRemotePeerConnect;
	unsigned int remotePeerConnectSubscriptionId;

	void TrackOnRemotePeerConnect()
	{
		auto callback = [&](uint32_t remotePeerId) {this->AddUnprocessedConnectedRemotePeer(remotePeerId); };
		remotePeerConnectSubscriptionId = peer->SubscribeToOnRemotePeerConnect(callback);
		isTrackingRemotePeerConnect = true;
	}

	void AddUnprocessedConnectedRemotePeer(uint32_t remotePeerId) { unprocessedConnectedRemotePeers.push(0); };

	NetLib::Client* GetPeerAsClient() const { assert(peer != nullptr); assert(peer->GetPeerType() == NetLib::PeerType::ClientMode); return static_cast<NetLib::Client*>(peer); }
	NetLib::Server* GetPeerAsServer() const { assert(peer != nullptr); assert(peer->GetPeerType() == NetLib::PeerType::ServerMode); return static_cast<NetLib::Server*>(peer); }
};
