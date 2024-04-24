#pragma once
#include "Peer.h"

class NetworkSystem
{
public:
	void Initialize(NetLib::PeerType type);
	void PreTick() const;
	void PosTick() const;
	void Release();

private:
	NetLib::Peer* _networkPeer;
};
