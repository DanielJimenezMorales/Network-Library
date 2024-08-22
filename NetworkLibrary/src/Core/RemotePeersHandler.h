#pragma once
#include <vector>
#include <queue>
#include <unordered_set>
#include <cstdint>

#include "RemotePeer.h"

namespace NetLib
{
	class Address;

	enum RemotePeersHandlerResult : uint8_t
	{
		RPH_ERROR = 0,
		RPH_SUCCESS = 1,
		RPH_FULL = 2,
		RPH_ALREADYEXIST = 3
	};

	//If the local peer does not receive any message from a remote peer during REMOTE_PEER_INACTIVITY_TIME seconds it will be considered inactive and it will
	//be disconnected with ConnectionFailedReasonType::CFR_TIMEOUT reason
	const float REMOTE_PEER_INACTIVITY_TIME = 5.0f;

	class RemotePeersHandler
	{
	public:
		RemotePeersHandler(unsigned int maxConnections);

		void TickRemotePeers(float elapsedTime);

		bool AddRemotePeer(const Address& addressInfo, uint16_t id, uint64_t clientSalt, uint64_t serverSalt);
		int FindFreeRemotePeerSlot() const;
		RemotePeer* GetRemotePeerFromAddress(const Address& address);
		RemotePeer* GetRemotePeerFromId(unsigned int id);
		bool IsRemotePeerAlreadyConnected(const Address& address) const;
		bool DoesRemotePeerIdExist(unsigned int id) const;
		RemotePeersHandlerResult IsRemotePeerAbleToConnect(const Address& address) const;

		std::unordered_set<RemotePeer*>::iterator GetValidRemotePeersIterator();
		std::unordered_set<RemotePeer*>::iterator GetValidRemotePeersPastTheEndIterator();

		void RemoveAllRemotePeers();
		bool RemoveRemotePeer(unsigned int remotePeerId);

		~RemotePeersHandler();

	private:
		int GetIndexFromId(unsigned int id) const;

		const unsigned int _maxConnections;
		std::vector<bool> _remotePeerSlots;
		std::vector<RemotePeer> _remotePeers;
		std::unordered_set<RemotePeer*> _validRemotePeers;
	};
}
