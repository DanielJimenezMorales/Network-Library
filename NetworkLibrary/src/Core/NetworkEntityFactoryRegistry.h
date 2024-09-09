#pragma once
#include "NumericTypes.h"

namespace NetLib
{
	class INetworkEntityFactory;
	class NetworkVariableChangesHandler;

	class NetworkEntityFactoryRegistry
	{
	public:
		NetworkEntityFactoryRegistry() : _networkEntityFactory(nullptr)
		{
		}

		void RegisterNetworkEntityFactory(INetworkEntityFactory* entityFactory);
		uint32 CreateNetworkEntity(uint32 entityType, uint32 networkEntityId, uint32 controlledByPeerId, float32 posX, float32 posY, NetworkVariableChangesHandler* networkVariableChangesHandler);
		void RemoveNetworkEntity(uint32 inGameId);

	private:
		//TODO Support multiple network entity factories depending on the entity_type
		INetworkEntityFactory* _networkEntityFactory;
	};
}
