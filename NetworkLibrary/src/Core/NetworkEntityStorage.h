#pragma once
#include <cstdint>
#include <unordered_map>

#include "INetworkEntity.h"

namespace NetLib
{
	const uint32_t INVALID_NETWORK_ENTITY_ID = 0;

	class NetworkEntityStorage
	{
	public:
		NetworkEntityStorage() : _nextNetworkEntityId(1) {};

		void AddNetworkEntity(INetworkEntity& networkEntity);
		bool RemoveNetworkEntity(INetworkEntity& networkEntity);
		std::unordered_map<uint32_t, INetworkEntity*>::const_iterator GetNetworkEntities() const;
		std::unordered_map<uint32_t, INetworkEntity*>::const_iterator GetPastToEndNetworkEntities() const;

	private:
		void CalculateNextNetworkEntityId();
		bool RemoveNetworkEntity(uint32_t networkEntityId);

		std::unordered_map<uint32_t, INetworkEntity*> _networkEntities;
		uint32_t _nextNetworkEntityId;
	};
}
