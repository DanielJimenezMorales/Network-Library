#pragma once
#include "NetworkVariable.h"
#include "NetworkVariableChangesHandler.h"

struct RemotePlayerControllerComponent
{
public:
	RemotePlayerControllerComponent(NetLib::NetworkVariableChangesHandler* networkVariableChangesHandler, uint32 networkEntityId) :
		networkEntityId(networkEntityId), posX(networkVariableChangesHandler, networkEntityId), posY(networkVariableChangesHandler, networkEntityId), rotationAngle(networkVariableChangesHandler, networkEntityId) {};

	uint32 networkEntityId;
	NetLib::NetworkVariable<float> posX;
	NetLib::NetworkVariable<float> posY;

	NetLib::NetworkVariable<float> rotationAngle;
};
