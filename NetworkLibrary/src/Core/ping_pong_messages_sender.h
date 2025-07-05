#pragma once
#include "numeric_types.h"

namespace NetLib
{
	class RemotePeer;

	static const float32 DEFAULT_PING_PONG_MESSAGE_FREQUENCY_SECONDS = 0.f;

	class PingPongMessagesSender
	{
		public:
			PingPongMessagesSender();

			void Update( float32 elapsed_time, RemotePeer& remote_peer );

		private:
			float32 _timeLeftUntilNextPingPongMessage;
			float32 _pingPongMessageFrequencySeconds;
	};
}
