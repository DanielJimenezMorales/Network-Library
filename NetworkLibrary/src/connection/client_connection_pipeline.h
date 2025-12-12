#pragma once
#include "connection/i_connection_pipeline.h"

namespace NetLib
{
	namespace Connection
	{
		class ClientConnectionPipeline : public IConnectionPipeline
		{
			public:
				virtual void ProcessConnection( PendingConnection& pending_connection, MessageFactory& message_factory,
				                                float32 elapsed_time ) override;
		};
	}
}
