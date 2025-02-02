#pragma once
#include "numeric_types.h"

namespace NetLib
{
	struct NetworkEntityCommunicationCallbacks;

	struct OnNetworkEntityCreateConfig
	{
			uint32 entityType;
			uint32 entityId;
			uint32 controlledByPeerId;

			float32 positionX;
			float32 positionY;
			NetworkEntityCommunicationCallbacks* communicationCallbacks;
	};
}
