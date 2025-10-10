#pragma once
#include "shared/component_configurations/player_controller_component_configuration.h"

#include "shared/player_simulation/player_state_configuration.h"

struct PlayerControllerComponent
{
	public:
		PlayerControllerComponent()
		    : stateConfiguration( 0, 0 )
		    , isWalking( false )
		    , isAiming( false )
		    , timeLeftUntilNextShot( 0.f )
		{
		}

		PlayerSimulation::PlayerStateConfiguration stateConfiguration;

		bool isWalking;
		bool isAiming;
		float32 timeLeftUntilNextShot;
};
