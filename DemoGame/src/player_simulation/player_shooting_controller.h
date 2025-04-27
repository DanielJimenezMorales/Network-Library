#pragma once
#include "numeric_types.h"

#include "Delegate.h"

class InputState;
struct PlayerState;
struct PlayerStateConfiguration;

class PlayerShootingController
{
	public:
		PlayerShootingController();

		bool Simulate( const InputState& inputs, const PlayerState& current_state, PlayerState& result_state,
		               float32 elapsed_time, const PlayerStateConfiguration& configuration );

		Common::Delegate<> OnShotPerformed;
};
