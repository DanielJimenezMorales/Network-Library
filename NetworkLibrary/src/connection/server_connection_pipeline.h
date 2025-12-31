#pragma once
#include "connection/i_connection_pipeline.h"

namespace NetLib
{
	class Message;
	class ConnectionChallengeResponseMessage;

	namespace Connection
	{
		class ServerConnectionPipeline : public IConnectionPipeline
		{
			public:
				ServerConnectionPipeline();
				virtual void ProcessConnection( PendingConnection& pending_connection, MessageFactory& message_factory,
				                                float32 elapsed_time ) override;

			private:
				void ProcessMessage( PendingConnection& pending_connection, const Message* message,
				                     MessageFactory& message_factory );
				void ProcessConnectionChallengeResponse( PendingConnection& pending_connection,
				                                         const ConnectionChallengeResponseMessage& message,
				                                         MessageFactory& message_factory );
				uint16 GenerateNextConnectionApprovedId();

				static constexpr uint16 SERVER_CONNECTION_ID = 0;

				uint16 _nextConnectionApprovedId;
		};
	}
}
