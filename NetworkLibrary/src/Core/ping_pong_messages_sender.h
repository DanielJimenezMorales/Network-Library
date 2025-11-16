#pragma once
#include "numeric_types.h"

namespace NetLib
{
	class RemotePeer;
	class MessageFactory;

	class PingPongMessagesSender
	{
		public:
			PingPongMessagesSender();

			void Update( float32 elapsed_time, RemotePeer& remote_peer, MessageFactory& message_factory );
	};
}
