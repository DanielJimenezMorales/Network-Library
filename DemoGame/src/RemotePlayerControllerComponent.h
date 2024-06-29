#pragma once
#include "NetworkVariable.h"
#include "NetworkVariableChangesHandler.h"

struct RemotePlayerControllerComponent
{
public:
	RemotePlayerControllerComponent(NetLib::NetworkVariableChangesHandler* networkVariableChangesHandler, uint32_t networkEntityId) :
		networkEntityId(networkEntityId), posX(networkVariableChangesHandler, networkEntityId), posY(networkVariableChangesHandler, networkEntityId) {};

	uint32_t networkEntityId;
	NetLib::NetworkVariable<float> posX;
	NetLib::NetworkVariable<float> posY;
};
