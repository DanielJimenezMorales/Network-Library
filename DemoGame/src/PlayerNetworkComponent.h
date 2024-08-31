#pragma once

#include "NetworkVariable.h"
#include "NetworkVariableChangesHandler.h"

struct PlayerNetworkComponent
{
public:
	PlayerNetworkComponent(NetLib::NetworkVariableChangesHandler* networkVariableChangesHandler, uint32 networkEntityId) :
		networkEntityId(networkEntityId), posX(networkVariableChangesHandler, networkEntityId), posY(networkVariableChangesHandler, networkEntityId) {};

	uint32 networkEntityId;
	NetLib::NetworkVariable<float> posX;
	NetLib::NetworkVariable<float> posY;
};
