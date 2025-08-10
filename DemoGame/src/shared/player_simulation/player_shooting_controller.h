#pragma once
#include "numeric_types.h"

#include "shared/player_simulation/player_simulation_events.h"

#include <vector>

class InputState;
struct PlayerState;
struct PlayerStateConfiguration;

class PlayerShootingController
{
	public:
		PlayerShootingController();

		bool Simulate( const InputState& inputs, const PlayerState& current_state, PlayerState& result_state,
		               float32 elapsed_time, const PlayerStateConfiguration& configuration,
		               std::vector< PlayerSimulation::EventType >& events_generated );
};
