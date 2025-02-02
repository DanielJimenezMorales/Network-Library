#pragma once
#include "PlayerControllerConfiguration.h"

#include "replication/network_variable.hpp"
#include "replication/network_variable_changes_handler.h"

struct PlayerControllerComponent
{
	public:
		PlayerControllerComponent() {}

		PlayerControllerComponent( NetLib::NetworkVariableChangesHandler* networkVariableChangesHandler,
		                           uint32 networkEntityId, const PlayerControllerConfiguration& configuration )
		    : configuration( configuration ){};

		PlayerControllerConfiguration configuration;
};
