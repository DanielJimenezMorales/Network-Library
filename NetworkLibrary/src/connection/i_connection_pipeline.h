#pragma once
#include "numeric_types.h"

#include <unordered_map>

namespace NetLib
{
	class PendingConnection;
	class MessageFactory;

	class IConnectionPipeline
	{
		public:
			IConnectionPipeline() = default;
			virtual ~IConnectionPipeline() = default;
			virtual void ProcessConnection( PendingConnection& pending_connection, MessageFactory& message_factory,
			                                float32 elapsed_time ) = 0;
	};
}
