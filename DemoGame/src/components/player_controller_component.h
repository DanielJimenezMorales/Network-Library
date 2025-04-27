#pragma once
#include "component_configurations/player_controller_component_configuration.h"

#include "replication/network_variable.hpp"
#include "replication/network_variable_changes_handler.h"

struct PlayerControllerComponent
{
	public:
		PlayerControllerComponent() {}

		PlayerControllerComponent( NetLib::NetworkVariableChangesHandler* networkVariableChangesHandler,
		                           uint32 networkEntityId, const PlayerControllerComponentConfiguration& configuration )
		    : movementSpeed( configuration.movementSpeed )
		    , fireRatePerSecond( configuration.fireRatePerSecond )
		    , fireRate( 1 / fireRatePerSecond )
		    , timeLeftUntilNextShot( 0.f )
		{
		}

		uint32 movementSpeed;
		// This value can't be higher than the game Tick Rate
		uint32 fireRatePerSecond;
		float32 fireRate;
		float32 timeLeftUntilNextShot;
};
