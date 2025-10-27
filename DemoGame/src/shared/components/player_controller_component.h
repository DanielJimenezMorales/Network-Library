#pragma once
#include "shared/player_simulation/player_state_configuration.h"
#include "shared/player_simulation/player_state.h"

struct PlayerControllerComponent
{
	public:
		PlayerControllerComponent()
		    : stateConfiguration( 0, 0 )
		    , state()
		{
		}

		PlayerSimulation::PlayerStateConfiguration stateConfiguration;
		PlayerSimulation::PlayerState state;
};
