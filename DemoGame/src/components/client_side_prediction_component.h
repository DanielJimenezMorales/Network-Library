#pragma once
#include "numeric_types.h"

#include "client_side_prediction_buffer_slot.h"

#include <vector>

struct ClientSidePredictionComponent
{
		ClientSidePredictionComponent()
		    : inputStatesBuffer()
		    , playerStatesBuffer()
		    , playerStatesReceivedFromServer()
		{
		}

		const uint32 MAX_PREDICTION_BUFFER_SIZE = 256;
		std::vector< InputState > inputStatesBuffer;
		std::vector< PlayerState > playerStatesBuffer;

		std::vector< PlayerState > playerStatesReceivedFromServer;
};
