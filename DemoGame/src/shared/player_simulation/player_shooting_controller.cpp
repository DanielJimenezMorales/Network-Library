#include "player_shooting_controller.h"

#include "raycaster.h"
#include "math_utils.h"

#include "shared/InputState.h"

#include "shared/player_simulation/player_state.h"
#include "shared/player_simulation/player_state_configuration.h"
#include "shared/player_simulation/simulation_events_handler.h"
#include "shared/player_simulation/simulation_events.h"

#include "logger.h"

namespace PlayerSimulation
{
	bool PlayerShootingController::Simulate( const InputState& inputs, const PlayerState& current_state,
	                                         PlayerState& result_state, float32 elapsed_time,
	                                         const PlayerStateConfiguration& configuration,
	                                         SimulationEventsHandler& simulation_events_handler )
	{
		// Update time left until next shot
		float32 newTimeLeftUntilNextShot = ( current_state.timeLeftUntilNextShot > elapsed_time )
		                                       ? current_state.timeLeftUntilNextShot - elapsed_time
		                                       : 0.f;

		if ( inputs.isShooting && inputs.isAiming && newTimeLeftUntilNextShot == 0.f )
		{
			newTimeLeftUntilNextShot = configuration.GetFireRate();
			simulation_events_handler.AddEvent( PlayerSimulation::ON_SHOT_PERFORMED );
		}

		result_state.timeLeftUntilNextShot = newTimeLeftUntilNextShot;
		return true;
	}
}
