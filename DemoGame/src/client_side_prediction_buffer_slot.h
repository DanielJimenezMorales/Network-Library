#pragma once
#include "numeric_types.h"

#include "InputState.h"

#include "player_simulation/player_state.h"

struct ClientSidePredictionBufferSlot
{
		ClientSidePredictionBufferSlot()
		    : isValid( false )
		    , elapsedTime( 0.f )
		    , inputState()
		    , playerState()
		{
		}

		bool isValid;
		float32 elapsedTime;
		InputState inputState;
		PlayerState playerState;
};
