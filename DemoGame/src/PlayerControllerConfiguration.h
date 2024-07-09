#pragma once
#include <cstdint>

struct PlayerControllerConfiguration
{
	PlayerControllerConfiguration() :movementSpeed(0) {}
	PlayerControllerConfiguration(const PlayerControllerConfiguration& other) : movementSpeed(other.movementSpeed)
	{
	}

	uint32_t movementSpeed;
};