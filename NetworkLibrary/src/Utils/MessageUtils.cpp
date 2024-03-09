#include <sstream>
#include <memory>

#include "MessageUtils.h"
#include "MessageFactory.h"
#include "Buffer.h"
#include "Logger.h"

void MessageUtils::ReadMessage(Buffer& buffer, Message** message)
{
	MessageFactory& messageFactory = MessageFactory::GetInstance();

	MessageType type = static_cast<MessageType>(buffer.ReadByte());

	switch (type)
	{
	case MessageType::ConnectionRequest:
		*message = messageFactory.LendMessage(MessageType::ConnectionRequest).release();
		break;
	case MessageType::ConnectionAccepted:
		*message = messageFactory.LendMessage(MessageType::ConnectionAccepted).release();
		break;
	case MessageType::ConnectionDenied:
		*message = messageFactory.LendMessage(MessageType::ConnectionDenied).release();
		break;
	case MessageType::ConnectionChallenge:
		*message = messageFactory.LendMessage(MessageType::ConnectionChallenge).release();
		break;
	case MessageType::ConnectionChallengeResponse:
		*message = messageFactory.LendMessage(MessageType::ConnectionChallengeResponse).release();
		break;
	case MessageType::Disconnection:
		*message = messageFactory.LendMessage(MessageType::Disconnection).release();
		break;
	case MessageType::InGame:
		*message = messageFactory.LendMessage(MessageType::InGame).release();
		break;
	case MessageType::InGameResponse:
		*message = messageFactory.LendMessage(MessageType::InGameResponse).release();
		break;
	default:
		std::stringstream ss;
		ss << "Can't read message of type MessageType = " << (int)type << ", ignoring it...";
		LOG_WARNING(ss.str());
	}

	if (*message == nullptr)
	{
		return;
	}

	(*message)->Read(buffer);
}
