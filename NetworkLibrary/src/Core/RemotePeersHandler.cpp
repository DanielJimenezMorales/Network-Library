#include <cassert>

#include "RemotePeersHandler.h"
#include "Address.h"
#include "Logger.h"
#include "Peer.h"

namespace NetLib
{
	RemotePeersHandler::RemotePeersHandler(unsigned int maxConnections) : _maxConnections(maxConnections)
	{
		_remotePeerSlots.reserve(_maxConnections);
		_remotePeers.reserve(_maxConnections);
		_validRemotePeers.reserve(_maxConnections);

		for (unsigned int i = 0; i < _maxConnections; ++i)
		{
			_remotePeerSlots.push_back(false); //You could use vector.resize in these ones.
			_remotePeers.emplace_back();
		}
	}

	void RemotePeersHandler::TickRemotePeers(float elapsedTime)
	{
		for (unsigned int i = 0; i < _maxConnections; ++i)
		{
			if (_remotePeerSlots[i])
			{
				_remotePeers[i].Tick(elapsedTime);

				//Start the disconnection process for those ones who are inactive
				if (_remotePeers[i].IsInactive())
				{
					//StartDisconnectingRemotePeer(i, true, ConnectionFailedReasonType::CFR_TIMEOUT);
				}
			}
		}
	}

	bool RemotePeersHandler::AddRemotePeer(const Address& addressInfo, uint16_t id, uint64_t dataPrefix)
	{
		//TODO Check for every single possible error scenario. Is already connected? Is it maxConnections full? etc. Return error code
		int slotIndex = FindFreeRemotePeerSlot();
		if (slotIndex == -1)
		{
			return false;
		}

		_remotePeerSlots[slotIndex] = true;
		_remotePeers[slotIndex].Connect(addressInfo.GetInfo(), id, REMOTE_PEER_INACTIVITY_TIME, dataPrefix);

		auto it = _validRemotePeers.insert(_remotePeers[slotIndex]);
		assert(it.second); //If the element was already there it means that we are trying to add it again. ERROR!!
		return true;
	}

	int RemotePeersHandler::FindFreeRemotePeerSlot() const
	{
		int freeIndex = -1;
		for (int i = 0; i < _maxConnections; ++i)
		{
			if (!_remotePeerSlots[i])
			{
				freeIndex = i;
				break;
			}
		}

		return freeIndex;
	}

	RemotePeer* RemotePeersHandler::GetRemotePeerFromAddress(const Address& address)
	{
		RemotePeer* result = nullptr;
		for (unsigned int i = 0; i < _maxConnections; ++i)
		{
			if (!_remotePeerSlots[i])
			{
				continue;
			}

			if (_remotePeers[i].GetAddress() == address)
			{
				result = &_remotePeers[i];
				break;
			}
		}

		return result;
	}

	int RemotePeersHandler::GetRemotePeerIndexFromAddress(const Address& address) const
	{
		int index = -1;
		for (unsigned int i = 0; i < _maxConnections; ++i)
		{
			if (!_remotePeerSlots[i])
			{
				continue;
			}

			if (_remotePeers[i].GetAddress() == address)
			{
				index = i;
				break;
			}
		}

		return index;
	}

	int RemotePeersHandler::GetRemotePeerIndex(const RemotePeer& remotePeer) const
	{
		int index = -1;
		for (unsigned int i = 0; i < _maxConnections; ++i)
		{
			if (_remotePeerSlots[i])
			{
				if (_remotePeers[i].IsAddressEqual(remotePeer.GetAddress()))
				{
					index = i;
					break;
				}
			}
		}

		return index;
	}

	bool RemotePeersHandler::IsRemotePeerAlreadyConnected(const Address& address) const
	{
		bool found = false;
		for (unsigned int i = 0; i < _maxConnections; ++i)
		{
			if (!_remotePeerSlots[i])
			{
				continue;
			}

			if (_remotePeers[i].GetAddress() == address)
			{
				found = true;
				break;
			}
		}

		return found;
	}

	void RemotePeersHandler::RemoveAllRemotePeers()
	{
		for (unsigned int i = 0; i < _maxConnections; ++i)
		{
			if (_remotePeerSlots[i])
			{
				RemoveRemotePeer(i);
			}
		}
	}

	void RemotePeersHandler::RemoveRemotePeer(unsigned int remotePeerIndex)
	{
		if (_remotePeerSlots[remotePeerIndex])
		{
			_remotePeerSlots[remotePeerIndex] = false;
			_remotePeers[remotePeerIndex].Disconnect();

			auto it = _validRemotePeers.find(_remotePeers[remotePeerIndex]);
			assert(it != _validRemotePeers.end()); //If it does not exist in the valid peers and we are trying to delete it, that is an ERROR!!
			_validRemotePeers.erase(it);
		}
	}

	RemotePeersHandler::~RemotePeersHandler()
	{
	}
}
