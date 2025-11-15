#include "message_utils.h"

#include "logger.h"

#include "core/buffer.h"

#include "communication/message_factory.h"

namespace NetLib
{
	std::unique_ptr< Message > MessageUtils::ReadMessage( MessageFactory& message_factory, Buffer& buffer )
	{
		MessageType type = static_cast< MessageType >( buffer.ReadByte() );
		std::unique_ptr< Message > message = nullptr;

		switch ( type )
		{
			case MessageType::ConnectionRequest:
				message = message_factory.LendMessage( MessageType::ConnectionRequest );
				break;
			case MessageType::ConnectionAccepted:
				message = message_factory.LendMessage( MessageType::ConnectionAccepted );
				break;
			case MessageType::ConnectionDenied:
				message = message_factory.LendMessage( MessageType::ConnectionDenied );
				break;
			case MessageType::ConnectionChallenge:
				message = message_factory.LendMessage( MessageType::ConnectionChallenge );
				break;
			case MessageType::ConnectionChallengeResponse:
				message = message_factory.LendMessage( MessageType::ConnectionChallengeResponse );
				break;
			case MessageType::Disconnection:
				message = message_factory.LendMessage( MessageType::Disconnection );
				break;
			case MessageType::TimeRequest:
				message = message_factory.LendMessage( MessageType::TimeRequest );
				break;
			case MessageType::TimeResponse:
				message = message_factory.LendMessage( MessageType::TimeResponse );
				break;
			case MessageType::Replication:
				message = message_factory.LendMessage( MessageType::Replication );
				break;
			case MessageType::Inputs:
				message = message_factory.LendMessage( MessageType::Inputs );
				break;
			case MessageType::PingPong:
				message = message_factory.LendMessage( MessageType::PingPong );
				break;
			default:
				LOG_WARNING( "Can't read message of type MessageType = %hhu. Ignoring it...", type );
		}

		if ( message != nullptr )
		{
			message->Read( buffer );
		}

		return std::move( message );
	}
} // namespace NetLib
