#pragma once
#include "numeric_types.h"

#include <unordered_map>

namespace NetLib
{
	class MessageFactory;

	namespace Connection
	{
		class PendingConnection;

		class IConnectionPipeline
		{
			public:
				IConnectionPipeline() = default;
				virtual ~IConnectionPipeline() = default;
				virtual void ProcessConnection( PendingConnection& pending_connection, MessageFactory& message_factory,
				                                float32 elapsed_time ) = 0;
		};
	}
}
