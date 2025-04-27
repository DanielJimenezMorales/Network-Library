#include "player_rotation_controller.h"

#include "InputState.h"

#include "components/transform_component.h"

#include "player_simulation/player_state.h"
#include "player_simulation/player_state_configuration.h"

static void UpdateLookAt( const InputState& inputs, const PlayerState& current_state, PlayerState& result_state,
                          float32 elapsed_time, const PlayerStateConfiguration& configuration )
{
	TransformComponent transform( current_state.position, current_state.rotationAngle );
	transform.LookAt( inputs.virtualMousePosition );
	result_state.rotationAngle = transform.GetRotationAngle();
}

bool PlayerRotationController::Simulate( const InputState& inputs, const PlayerState& current_state,
                                         PlayerState& result_state, float32 elapsed_time,
                                         const PlayerStateConfiguration& configuration ) const
{
	UpdateLookAt( inputs, current_state, result_state, elapsed_time, configuration );
	return true;
}
