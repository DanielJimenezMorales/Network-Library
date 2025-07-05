#pragma once
#include "numeric_types.h"

namespace NetLib
{
	class RemotePeer;

	class PingPongMessagesSender
	{
		public:
			PingPongMessagesSender();

			void Update( float32 elapsed_time, RemotePeer& remote_peer );
	};
}
