#pragma once
#include <cstdint>
#include <unordered_map>

namespace NetLib
{
	class NetworkEntityStorage
	{
	public:
		NetworkEntityStorage() = default;

		bool HasNetworkEntityId(uint32_t networkEntityId) const;
		bool TryGetNetworkEntityFromId(uint32_t entityId, uint32_t& gameEntityId);
		void AddNetworkEntity(uint32_t networkEntityId, uint32_t gameEntityId);
		bool RemoveNetworkEntity(uint32_t networkEntityId);
		std::unordered_map<uint32_t, uint32_t>::const_iterator GetNetworkEntities() const;
		std::unordered_map<uint32_t, uint32_t>::const_iterator GetPastToEndNetworkEntities() const;

	private:
		std::unordered_map<uint32_t, uint32_t> _networkEntityIdToGameEntityIdMap;
	};
}
