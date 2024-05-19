#include <cassert>
#include "NetworkEntityStorage.h"

namespace NetLib
{
	bool NetworkEntityStorage::HasNetworkEntityId(uint32_t networkEntityId) const
	{
		auto it = _networkEntityIdToGameEntityIdMap.find(networkEntityId);
		return it != _networkEntityIdToGameEntityIdMap.cend();
	}

	bool NetworkEntityStorage::TryGetNetworkEntityFromId(uint32_t entityId, uint32_t& gameEntityId)
	{
		auto it = _networkEntityIdToGameEntityIdMap.find(entityId);

		if (it == _networkEntityIdToGameEntityIdMap.cend())
		{
			return false;
		}

		gameEntityId = it->second;
		return true;
	}

	void NetworkEntityStorage::AddNetworkEntity(uint32_t networkEntityId, uint32_t gameEntityId)
	{
		assert(_networkEntityIdToGameEntityIdMap.find(networkEntityId) == _networkEntityIdToGameEntityIdMap.cend());

		_networkEntityIdToGameEntityIdMap[networkEntityId] = gameEntityId;
	}

	std::unordered_map<uint32_t, uint32_t>::const_iterator NetworkEntityStorage::GetNetworkEntities() const
	{
		return _networkEntityIdToGameEntityIdMap.cbegin();
	}

	std::unordered_map<uint32_t, uint32_t>::const_iterator NetworkEntityStorage::GetPastToEndNetworkEntities() const
	{
		return _networkEntityIdToGameEntityIdMap.cend();
	}

	bool NetworkEntityStorage::RemoveNetworkEntity(uint32_t networkEntityId)
	{
		auto it = _networkEntityIdToGameEntityIdMap.find(networkEntityId);

		if (it == _networkEntityIdToGameEntityIdMap.end())
		{
			return false;
		}

		_networkEntityIdToGameEntityIdMap.erase(it);
		return true;
	}
}
