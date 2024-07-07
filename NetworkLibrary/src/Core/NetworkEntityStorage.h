#pragma once
#include <cstdint>
#include <unordered_map>

namespace NetLib
{
	struct NetworkEntityData
	{
		NetworkEntityData() = default;
		NetworkEntityData(uint32_t entityType, uint32_t id, uint32_t gameEntityId, uint32_t controlledByPeerId) : entityType(entityType), id(id), inGameId(gameEntityId), controlledByPeerId(controlledByPeerId)
		{
		}

		uint32_t entityType;
		uint32_t id;
		uint32_t inGameId;
		uint32_t controlledByPeerId;
	};

	class NetworkEntityStorage
	{
	public:
		NetworkEntityStorage() = default;

		bool HasNetworkEntityId(uint32_t networkEntityId) const;
		bool TryGetNetworkEntityFromId(uint32_t entityId, NetworkEntityData& gameEntityId);
		void AddNetworkEntity(uint32_t entityType, uint32_t networkEntityId, uint32_t controlledByPeerId, uint32_t gameEntityId);
		bool RemoveNetworkEntity(uint32_t networkEntityId);
		std::unordered_map<uint32_t, NetworkEntityData>::const_iterator GetNetworkEntities() const;
		std::unordered_map<uint32_t, NetworkEntityData>::const_iterator GetPastToEndNetworkEntities() const;

	private:
		std::unordered_map<uint32_t, NetworkEntityData> _networkEntityIdToDataMap;
	};
}
