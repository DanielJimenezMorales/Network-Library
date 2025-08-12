#pragma once
#include "numeric_types.h"

#include <vector>

class InputState;
struct PlayerState;
struct PlayerStateConfiguration;

namespace PlayerSimulation
{
	class SimulationEventsHandler;

	class PlayerMovementController
	{
		public:
			PlayerMovementController() = default;

			bool Simulate( const InputState& inputs, const PlayerState& current_state, PlayerState& result_state,
			               float32 elapsed_time, const PlayerStateConfiguration& configuration,
			               SimulationEventsHandler& simulation_events_handler ) const;
	};
}
