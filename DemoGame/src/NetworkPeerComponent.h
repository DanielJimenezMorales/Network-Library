#pragma once
#include <vector>
#include "Peer.h"

class INetworkEntityFactory;

struct NetworkPeerComponent
{
	//TODO free this memory correctly
	NetLib::Peer* peer;
	std::vector<INetworkEntityFactory*> networkEntityFactories;
};
