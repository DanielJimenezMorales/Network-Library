#pragma once
#include "shared/component_configurations/player_controller_component_configuration.h"

#include "shared/player_simulation/player_state_configuration.h"

struct PlayerControllerComponent
{
	public:
		PlayerControllerComponent()
		    : stateConfiguration( 0, 0 )
		    , movementDirection( 0.f, 0.f )
		    , isWalking( false )
		    , isAiming( false )
		    , timeLeftUntilNextShot( 0.f )
		{
		}

		PlayerSimulation::PlayerStateConfiguration stateConfiguration;

		Vec2f movementDirection;
		bool isWalking;
		bool isAiming;
		float32 timeLeftUntilNextShot;
};
