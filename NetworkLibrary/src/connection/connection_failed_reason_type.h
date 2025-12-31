#pragma once
#include "numeric_types.h"

namespace NetLib
{
	namespace Connection
	{
		enum class ConnectionFailedReasonType : uint8
		{
			UNKNOWN = 0,                 // Unexpect
			TIMEOUT = 1,                 // The peer is inactive
			SERVER_FULL = 2,             // The server can't handle more connections, it has reached its maximum
			PEER_SHUT_DOWN = 3,          // The peer has shut down its Network system
			CONNECTION_TIMEOUT = 4,      // The in process connection has taken too long
			WRONG_CHALLENGE_RESPONSE = 5 // The challenge response from the client didn't match the expected one
		};
	}
}
