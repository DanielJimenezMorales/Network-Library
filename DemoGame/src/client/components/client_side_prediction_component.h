#pragma once
#include "numeric_types.h"

#include "shared/InputState.h"
#include "shared/player_simulation/player_state.h"

#include <vector>

struct ClientSidePredictionComponent
{
		ClientSidePredictionComponent()
		    : inputStatesBuffer()
		    , elapsedTimeBuffer()
		    , resultedPlayerStatesBuffer()
		    , lastTickProcessedFromServer( 0 )
		    , playerStatesReceivedFromServer()
		    , isPendingPlayerStateFromServer( false )
		{
		}

		uint32 ConvertTickToBufferSlotIndex( uint32 tick ) const { return tick % MAX_PREDICTION_BUFFER_SIZE; }

		const uint32 MAX_PREDICTION_BUFFER_SIZE = 256;
		std::vector< InputState > inputStatesBuffer;
		std::vector< float32 > elapsedTimeBuffer;
		std::vector< PlayerState > resultedPlayerStatesBuffer;

		uint32 lastTickProcessedFromServer;
		std::vector< PlayerState > playerStatesReceivedFromServer;
		bool isPendingPlayerStateFromServer;
};
