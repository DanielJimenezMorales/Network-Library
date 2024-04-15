#pragma once
#include <cstdint>
#include <unordered_map>

#include "NetworkEntityFactory.h"

namespace NetLib
{
	class ReplicationObjectRegistry
	{
	public:
		ReplicationObjectRegistry() {};
		bool RegisterNetworkEntityFactory(NetworkEntityFactory* entityFactory, uint32_t entityType);

		INetworkEntity* CreateObjectOfType(uint32_t entityType);
		void DestroyObjectOfType(INetworkEntity& networkEntityToDestroy);

	private:
		NetworkEntityFactory* GetNetworkEntityFactory(uint32_t entityType);
		bool IsEntityFactoryRegistered(uint32_t entityType) const;

		std::unordered_map<uint32_t, NetworkEntityFactory*> _entityFactories;
	};
}
