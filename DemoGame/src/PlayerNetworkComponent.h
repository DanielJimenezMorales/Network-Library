#pragma once

#include "NetworkVariable.h"
#include "NetworkVariableChangesHandler.h"

struct PlayerNetworkComponent
{
public:
	PlayerNetworkComponent(NetLib::NetworkVariableChangesHandler* networkVariableChangesHandler, uint32_t networkEntityId) :
		networkEntityId(networkEntityId), posX(networkVariableChangesHandler, networkEntityId, 1), posY(networkVariableChangesHandler, networkEntityId, 2) {};

	uint32_t networkEntityId;
	NetLib::NetworkVariable<float> posX;
	NetLib::NetworkVariable<float> posY;
};
