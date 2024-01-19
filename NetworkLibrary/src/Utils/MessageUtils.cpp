#include "MessageUtils.h"
#include "MessageFactory.h"
#include "Buffer.h"

void MessageUtils::ReadMessage(Buffer& buffer, Message** message)
{
	MessageFactory* messageFactory = MessageFactory::GetInstance();
	if (messageFactory == nullptr)
	{
		return;
	}

	MessageType type = static_cast<MessageType>(buffer.ReadByte());

	switch (type)
	{
	case MessageType::ConnectionRequest:
		*message = messageFactory->LendMessage(MessageType::ConnectionRequest);
		break;
	case MessageType::ConnectionAccepted:
		*message = messageFactory->LendMessage(MessageType::ConnectionAccepted);
		break;
	case MessageType::ConnectionDenied:
		*message = messageFactory->LendMessage(MessageType::ConnectionDenied);
		break;
	case MessageType::ConnectionChallenge:
		*message = messageFactory->LendMessage(MessageType::ConnectionChallenge);
		break;
	case MessageType::ConnectionChallengeResponse:
		*message = messageFactory->LendMessage(MessageType::ConnectionChallengeResponse);
		break;
	case MessageType::Disconnection:
		*message = messageFactory->LendMessage(MessageType::Disconnection);
		break;
	}

	if (*message == nullptr)
	{
		return;
	}

	(*message)->Read(buffer);
}
