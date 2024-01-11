#include "MessageUtils.h"
#include "Message.h"
#include "Buffer.h"

void MessageUtils::ReadMessage(Buffer& buffer, Message** message)
{
	MessageType type = static_cast<MessageType>(buffer.ReadByte());

	switch (type)
	{
	case MessageType::ConnectionRequest:
		*message = new ConnectionRequestMessage();
		break;
	case MessageType::ConnectionAccepted:
		*message = new ConnectionAcceptedMessage();
		break;
	case MessageType::ConnectionDenied:
		*message = new ConnectionDeniedMessage();
		break;
	case MessageType::ConnectionChallenge:
		*message = new ConnectionChallengeMessage();
		break;
	case MessageType::ConnectionChallengeResponse:
		*message = new ConnectionChallengeResponseMessage();
		break;
	case MessageType::Disconnection:
		*message = new DisconnectionMessage();
		break;
	}

	(*message)->Read(buffer);
}
