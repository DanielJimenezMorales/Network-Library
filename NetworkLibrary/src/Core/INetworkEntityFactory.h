#pragma once
#include <cstdint>

namespace NetLib
{
	class INetworkEntityFactory
	{
	public:
		virtual int CreateNetworkEntityObject(uint32_t networkEntityType, uint32_t networkEntityId, float posX, float posY) = 0;
		virtual void DestroyNetworkEntityObject(uint32_t gameEntity) = 0;
	};
}
