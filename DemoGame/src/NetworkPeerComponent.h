#pragma once
#include <cassert>
#include <vector>
#include "Peer.h"
#include "Client.h"
#include "Server.h"

class INetworkEntityFactory;

struct NetworkPeerComponent
{
	//TODO free this memory correctly
	NetLib::Peer* peer;
	std::vector<INetworkEntityFactory*> networkEntityFactories;

	NetLib::Client* GetPeerAsClient() const { assert(peer != nullptr); assert(peer->GetPeerType() == NetLib::PeerType::ClientMode); return static_cast<NetLib::Client*>(peer); }
	NetLib::Server* GetPeerAsServer() const { assert(peer != nullptr); assert(peer->GetPeerType() == NetLib::PeerType::ServerMode); return static_cast<NetLib::Server*>(peer); }
};
