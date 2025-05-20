#pragma once
#include "numeric_types.h"

class InputState;
struct PlayerState;
struct PlayerStateConfiguration;

class PlayerRotationController
{
	public:
		bool Simulate( const InputState& inputs, const PlayerState& current_state, PlayerState& result_state,
		               float32 elapsed_time, const PlayerStateConfiguration& configuration ) const;
};
