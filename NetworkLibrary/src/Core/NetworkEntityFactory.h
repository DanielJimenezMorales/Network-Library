#pragma once
#include "INetworkEntity.h"

namespace NetLib
{
	class NetworkEntityFactory
	{
	public:
		virtual INetworkEntity& Create() = 0;
		virtual void Destroy(INetworkEntity* networkEntity) = 0;
	};
}
