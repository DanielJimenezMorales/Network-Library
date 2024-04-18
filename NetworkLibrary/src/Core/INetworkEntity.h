#pragma once
#include <cstdint>

namespace NetLib
{
	class INetworkEntity
	{
	public:
		virtual uint32_t GetEntityId() const = 0;
		virtual void SetEntityId(uint32_t entityId) = 0;
		virtual uint32_t GetEntityType() const = 0;

		virtual void NetworkEntityCreate() = 0;
		virtual void NetworkEntityDestroy() = 0;
	};
}
