#pragma once
#include "replication/network_variable.hpp"
#include "replication/network_variable_changes_handler.h"

struct RemotePlayerControllerComponent
{
	public:
		RemotePlayerControllerComponent()
		    : networkEntityId( 0 )
		{
		}

		RemotePlayerControllerComponent( NetLib::NetworkVariableChangesHandler* networkVariableChangesHandler,
		                                 uint32 networkEntityId )
		    : networkEntityId( networkEntityId )
		{
		}

		uint32 networkEntityId;
};
