#pragma once
#include "numeric_types.h"

class InputState;

namespace PlayerSimulation
{
	class SimulationEventsHandler;
	struct PlayerState;
	struct PlayerStateConfiguration;

	class PlayerAimingController
	{
		public:
			PlayerAimingController() = default;

			bool Simulate( const InputState& inputs, const PlayerState& current_state, PlayerState& result_state,
			               float32 elapsed_time, const PlayerStateConfiguration& configuration,
			               SimulationEventsHandler& simulation_events_handler );
	};
}
