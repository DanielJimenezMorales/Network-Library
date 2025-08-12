#include "player_movement_controller.h"

#include "shared/InputState.h"

#include "shared/player_simulation/player_state.h"
#include "shared/player_simulation/player_state_configuration.h"
#include "shared/player_simulation/simulation_events_handler.h"
#include "shared/player_simulation/player_simulation_events.h"

namespace PlayerSimulation
{
	static bool IsMoving( const InputState& inputs )
	{
		return ( inputs.movement.X() != 0 || inputs.movement.Y() != 0 );
	}

	static void UpdatePosition( const InputState& inputs, const PlayerState& current_state, PlayerState& result_state,
	                            float32 elapsed_time, const PlayerStateConfiguration& configuration )
	{
		const float32 movement_speed_multiplied_by_elapsed_time = configuration.GetMovementSpeed() * elapsed_time;

		Vec2f position_delta( 0.f, 0.f );
		position_delta.X( inputs.movement.X() * movement_speed_multiplied_by_elapsed_time );
		position_delta.Y( inputs.movement.Y() * movement_speed_multiplied_by_elapsed_time );

		result_state.position = current_state.position + position_delta;
	}

	bool PlayerMovementController::Simulate( const InputState& inputs, const PlayerState& current_state,
	                                         PlayerState& result_state, float32 elapsed_time,
	                                         const PlayerStateConfiguration& configuration,
	                                         SimulationEventsHandler& simulation_events_handler ) const
	{
		if ( IsMoving( inputs ) )
		{
			UpdatePosition( inputs, current_state, result_state, elapsed_time, configuration );
		}
		else
		{
			result_state.position = current_state.position;
		}

		return true;
	}
} // namespace PlayerSimulation