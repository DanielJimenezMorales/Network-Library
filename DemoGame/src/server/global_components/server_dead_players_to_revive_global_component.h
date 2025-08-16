#pragma once
#include "numeric_types.h"

#include <vector>

struct DeadPlayerEntry
{
		DeadPlayerEntry()
		    : remotePeerId( 0 )
		    , timeLeft( 0.0f )
		{
		}

		uint32 remotePeerId;
		float32 timeLeft;
};

struct ServerDeadPlayersToReviveGlobalComponent
{
		std::vector< DeadPlayerEntry > deadPlayersToRevive;
};
