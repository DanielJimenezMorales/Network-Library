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
		    : movementSpeed( configuration.movementSpeed ){};

		uint32 movementSpeed;
};
