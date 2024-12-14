#include "message_utils.h"

#include "Buffer.h"
#include "Logger.h"

#include "communication/message_factory.h"

namespace NetLib
{
	std::unique_ptr<Message> MessageUtils::ReadMessage(Buffer& buffer)
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();

		MessageType type = static_cast<MessageType>(buffer.ReadByte());
		std::unique_ptr<Message> message = nullptr;

		switch (type)
		{
		case MessageType::ConnectionRequest:
			message = messageFactory.LendMessage(MessageType::ConnectionRequest);
			break;
		case MessageType::ConnectionAccepted:
			message = messageFactory.LendMessage(MessageType::ConnectionAccepted);
			break;
		case MessageType::ConnectionDenied:
			message = messageFactory.LendMessage(MessageType::ConnectionDenied);
			break;
		case MessageType::ConnectionChallenge:
			message = messageFactory.LendMessage(MessageType::ConnectionChallenge);
			break;
		case MessageType::ConnectionChallengeResponse:
			message = messageFactory.LendMessage(MessageType::ConnectionChallengeResponse);
			break;
		case MessageType::Disconnection:
			message = messageFactory.LendMessage(MessageType::Disconnection);
			break;
		case MessageType::TimeRequest:
			message = messageFactory.LendMessage(MessageType::TimeRequest);
			break;
		case MessageType::TimeResponse:
			message = messageFactory.LendMessage(MessageType::TimeResponse);
			break;
		case MessageType::Replication:
			message = messageFactory.LendMessage(MessageType::Replication);
			break;
		case MessageType::Inputs:
			message = messageFactory.LendMessage(MessageType::Inputs);
			break;
		default:
			LOG_WARNING("Can't read message of type MessageType = %hhu. Ignoring it...", type);
		}

		if (message != nullptr)
		{
			message->Read(buffer);
		}

		return std::move(message);
	}
}
