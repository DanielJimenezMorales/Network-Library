#pragma once
#include <cstdint>

namespace NetLib
{
	class INetworkEntity
	{
	public:
		virtual uint32_t GetEntityId() const = 0;
		virtual void SetEntityId(uint32_t entityId) = 0;
	};
}
