#pragma once
#include "numeric_types.h"

namespace NetLib
{
	enum ConnectionFailedReasonType : uint8
	{
		CFR_UNKNOWN = 0,                 // Unexpect
		CFR_TIMEOUT = 1,                 // The peer is inactive
		CFR_SERVER_FULL = 2,             // The server can't handle more connections, it has reached its maximum
		CFR_PEER_SHUT_DOWN = 3,          // The peer has shut down its Network system
		CFR_CONNECTION_TIMEOUT = 4,      // The in process connection has taken too long
		CFR_WRONG_CHALLENGE_RESPONSE = 5 // The challenge response from the client didn't match the expected one
	};
}
