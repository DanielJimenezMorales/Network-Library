#pragma once
#include "numeric_types.h"

#include <vector>

class InputState;

namespace PlayerSimulation
{
	class SimulationEventsHandler;
	struct PlayerState;
	struct PlayerStateConfiguration;

	class PlayerRotationController
	{
		public:
			PlayerRotationController() = default;

			bool Simulate( const InputState& inputs, const PlayerState& current_state, PlayerState& result_state,
			               float32 elapsed_time, const PlayerStateConfiguration& configuration,
			               SimulationEventsHandler& simulation_events_handler ) const;
	};
}
