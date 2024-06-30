#pragma once
#include "NetworkVariable.h"
#include "NetworkVariableChangesHandler.h"

struct PlayerControllerConfiguration
{
	PlayerControllerConfiguration() :movementSpeed(0) {}
	PlayerControllerConfiguration(const PlayerControllerConfiguration& other) : movementSpeed(other.movementSpeed)
	{
	}

	uint32_t movementSpeed;
};

struct PlayerControllerComponent
{
public:
	PlayerControllerComponent(NetLib::NetworkVariableChangesHandler* networkVariableChangesHandler, uint32_t networkEntityId, const PlayerControllerConfiguration& configuration) :
		networkEntityId(networkEntityId), posX(networkVariableChangesHandler, networkEntityId), posY(networkVariableChangesHandler, networkEntityId), configuration(configuration) {};

	const PlayerControllerConfiguration configuration;
	uint32_t networkEntityId;
	NetLib::NetworkVariable<float> posX;
	NetLib::NetworkVariable<float> posY;
};
