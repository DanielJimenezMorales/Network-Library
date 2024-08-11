#pragma once
#include "NetworkVariable.h"
#include "NetworkVariableChangesHandler.h"
#include "PlayerControllerConfiguration.h"

struct PlayerControllerComponent
{
public:
	PlayerControllerComponent(NetLib::NetworkVariableChangesHandler* networkVariableChangesHandler, uint32_t networkEntityId, const PlayerControllerConfiguration& configuration) :
		networkEntityId(networkEntityId), posX(networkVariableChangesHandler, networkEntityId), posY(networkVariableChangesHandler, networkEntityId), rotationAngle(networkVariableChangesHandler, networkEntityId), configuration(configuration) {};

	const PlayerControllerConfiguration configuration;
	uint32_t networkEntityId;
	NetLib::NetworkVariable<float> posX;
	NetLib::NetworkVariable<float> posY;

	NetLib::NetworkVariable<float> rotationAngle;
};
