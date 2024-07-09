#pragma once
#include <cstdint>

struct NetworkEntityComponent
{
	NetworkEntityComponent(uint32_t entityId, uint32_t controlledByPeerId) : networkEntityId(entityId), controlledByPeerId(controlledByPeerId)
	{
	}

	uint32_t networkEntityId;
	uint32_t controlledByPeerId;
};
