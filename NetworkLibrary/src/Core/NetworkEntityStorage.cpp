#include "NetworkEntityStorage.h"

namespace NetLib
{
	void NetworkEntityStorage::AddNetworkEntity(INetworkEntity& networkEntity)
	{
		uint32_t id = _nextNetworkEntityId;
		networkEntity.SetEntityId(id);
		_networkEntities[id] = &networkEntity;

		CalculateNextNetworkEntityId();
	}

	bool NetworkEntityStorage::RemoveNetworkEntity(INetworkEntity& networkEntity)
	{
		return RemoveNetworkEntity(networkEntity.GetEntityId());
	}

	std::unordered_map<uint32_t, INetworkEntity*>::const_iterator NetworkEntityStorage::GetNetworkEntities() const
	{
		return _networkEntities.cbegin();
	}

	std::unordered_map<uint32_t, INetworkEntity*>::const_iterator NetworkEntityStorage::GetPastToEndNetworkEntities() const
	{
		return _networkEntities.cend();
	}

	void NetworkEntityStorage::CalculateNextNetworkEntityId()
	{
		++_nextNetworkEntityId;

		if (_nextNetworkEntityId == INVALID_NETWORK_ENTITY_ID)
		{
			++_nextNetworkEntityId;
		}
	}

	bool NetworkEntityStorage::RemoveNetworkEntity(uint32_t networkEntityId)
	{
		auto it = _networkEntities.find(networkEntityId);

		if (it == _networkEntities.end())
		{
			return false;
		}

		_networkEntities.erase(it);
		return true;
	}
}
