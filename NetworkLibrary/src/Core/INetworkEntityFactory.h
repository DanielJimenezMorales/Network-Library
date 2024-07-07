#pragma once
#include <cstdint>

namespace NetLib
{
	class NetworkVariableChangesHandler;

	class INetworkEntityFactory
	{
	public:
		virtual int CreateNetworkEntityObject(uint32_t networkEntityType, uint32_t networkEntityId, uint32_t controlledByPeerId, float posX, float posY,
			NetworkVariableChangesHandler* networkVariableChangeHandler) = 0;
		virtual void DestroyNetworkEntityObject(uint32_t gameEntity) = 0;
	};
}
