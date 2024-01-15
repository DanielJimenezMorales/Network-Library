#include "MessageUtils.h"
#include "MessageFactory.h"
#include "Buffer.h"

void MessageUtils::ReadMessage(Buffer& buffer, Message** message)
{
	MessageType type = static_cast<MessageType>(buffer.ReadByte());

	switch (type)
	{
	case MessageType::ConnectionRequest:
		*message = MessageFactory::GetMessage(MessageType::ConnectionRequest);
		break;
	case MessageType::ConnectionAccepted:
		*message = MessageFactory::GetMessage(MessageType::ConnectionAccepted);
		break;
	case MessageType::ConnectionDenied:
		*message = MessageFactory::GetMessage(MessageType::ConnectionDenied);
		break;
	case MessageType::ConnectionChallenge:
		*message = MessageFactory::GetMessage(MessageType::ConnectionChallenge);
		break;
	case MessageType::ConnectionChallengeResponse:
		*message = MessageFactory::GetMessage(MessageType::ConnectionChallengeResponse);
		break;
	case MessageType::Disconnection:
		*message = MessageFactory::GetMessage(MessageType::Disconnection);
		break;
	}

	if (*message == nullptr)
	{
		return;
	}

	(*message)->Read(buffer);
}
