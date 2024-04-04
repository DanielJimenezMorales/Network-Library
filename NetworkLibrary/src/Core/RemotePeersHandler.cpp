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

		auto it = _validRemotePeers.insert(&(_remotePeers[slotIndex]));
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

	RemotePeer* RemotePeersHandler::GetRemotePeerFromId(unsigned int id)
	{
		RemotePeer* result = nullptr;
		for (unsigned int i = 0; i < _maxConnections; ++i)
		{
			if (!_remotePeerSlots[i])
			{
				continue;
			}

			if (_remotePeers[i].GetClientIndex() == id)
			{
				result = &_remotePeers[i];
				break;
			}
		}

		return result;
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

	bool RemotePeersHandler::DoesRemotePeerIdExist(unsigned int id) const
	{
		bool result = false;

		if (GetIndexFromId(id) != -1)
		{
			result = true;
		}

		return result;
	}

	std::unordered_set<RemotePeer*>::iterator RemotePeersHandler::GetValidRemotePeersIterator()
	{
		return _validRemotePeers.begin();
	}

	std::unordered_set<RemotePeer*>::iterator RemotePeersHandler::GetValidRemotePeersPastTheEndIterator()
	{
		return _validRemotePeers.end();
	}

	void RemotePeersHandler::RemoveAllRemotePeers()
	{
		for (unsigned int i = 0; i < _maxConnections; ++i)
		{
			if (_remotePeerSlots[i])
			{
				RemoveRemotePeer(_remotePeers[i].GetClientIndex());
			}
		}
	}

	bool RemotePeersHandler::RemoveRemotePeer(unsigned int remotePeerId)
	{
		int index = GetIndexFromId(remotePeerId);
		if (index != -1)
		{
			_remotePeerSlots[index] = false;
			_remotePeers[index].Disconnect();

			auto it = _validRemotePeers.find(&(_remotePeers[index]));
			assert(it != _validRemotePeers.end()); //If it does not exist in the valid peers and we are trying to delete it, that is an ERROR!!
			_validRemotePeers.erase(it);

			return true;
		}

		return false;
	}

	int RemotePeersHandler::GetIndexFromId(unsigned int id) const
	{
		int index = -1;
		for (unsigned int i = 0; i < _maxConnections; ++i)
		{
			if (_remotePeerSlots[i])
			{
				if (_remotePeers[i].GetClientIndex() == id)
				{
					index = i;
					break;
				}
			}
		}

		return index;
	}

	RemotePeersHandler::~RemotePeersHandler()
	{
	}
}
