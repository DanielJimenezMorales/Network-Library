#include "player_aiming_controller.h"

#include "math_utils.h"

#include "shared/InputState.h"

#include "shared/player_simulation/player_state.h"
#include "shared/player_simulation/player_state_configuration.h"

#include "logger.h"

namespace PlayerSimulation
{
	bool PlayerAimingController::Simulate( const InputState& inputs, const PlayerState& current_state,
	                                       PlayerState& result_state, float32 elapsed_time,
	                                       const PlayerStateConfiguration& configuration,
	                                       SimulationEventsHandler& simulation_events_handler )
	{
		result_state.isAiming = inputs.isAiming;
		return true;
	}
}
