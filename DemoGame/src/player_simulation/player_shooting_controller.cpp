#include "player_shooting_controller.h"

#include "InputState.h"
#include "raycaster.h"
#include "math_utils.h"

#include "player_simulation/player_state.h"
#include "player_simulation/player_state_configuration.h"

PlayerShootingController::PlayerShootingController()
{
}

bool PlayerShootingController::Simulate( const InputState& inputs, const PlayerState& current_state,
                                         PlayerState& result_state, float32 elapsed_time,
                                         const PlayerStateConfiguration& configuration )
{
	// Update time left until next shot
	float32 newTimeLeftUntilNextShot = ( current_state.timeLeftUntilNextShot > elapsed_time )
	                                       ? current_state.timeLeftUntilNextShot - elapsed_time
	                                       : 0.f;

	if ( inputs.isShooting && newTimeLeftUntilNextShot == 0.f )
	{
		newTimeLeftUntilNextShot = configuration.GetFireRate();

		OnShotPerformed.Execute();
	}

	result_state.timeLeftUntilNextShot = newTimeLeftUntilNextShot;
	return true;
}
