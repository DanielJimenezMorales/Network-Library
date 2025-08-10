#include "player_shooting_controller.h"

#include "raycaster.h"
#include "math_utils.h"

#include "shared/InputState.h"

#include "shared/player_simulation/player_state.h"
#include "shared/player_simulation/player_state_configuration.h"

PlayerShootingController::PlayerShootingController()
{
}

bool PlayerShootingController::Simulate( const InputState& inputs, const PlayerState& current_state,
                                         PlayerState& result_state, float32 elapsed_time,
                                         const PlayerStateConfiguration& configuration,
                                         std::vector< PlayerSimulation::EventType >& events_generated )
{
	// Update time left until next shot
	float32 newTimeLeftUntilNextShot = ( current_state.timeLeftUntilNextShot > elapsed_time )
	                                       ? current_state.timeLeftUntilNextShot - elapsed_time
	                                       : 0.f;

	if ( inputs.isShooting && newTimeLeftUntilNextShot == 0.f )
	{
		newTimeLeftUntilNextShot = configuration.GetFireRate();
		events_generated.push_back( PlayerSimulation::ON_SHOT_PERFORMED );
	}

	result_state.timeLeftUntilNextShot = newTimeLeftUntilNextShot;
	return true;
}
