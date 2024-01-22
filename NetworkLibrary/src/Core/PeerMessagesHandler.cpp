#include <cassert>

#include "PeerMessagesHandler.h"
#include "Message.h"
#include "MessageFactory.h"

PeerMessagesHandler::PeerMessagesHandler()
{
	_pendingMessages.reserve(5);
}

bool PeerMessagesHandler::AddMessage(Message* message)
{
	_pendingMessages.push_back(message);
	return true;
}

Message* PeerMessagesHandler::GetAMessage()
{
	if (!ArePendingMessages())
	{
		return nullptr;
	}

	Message* message = _pendingMessages[0];
	_pendingMessages.erase(_pendingMessages.begin());

	_sentMessages.push(message);
	return message;
}

void PeerMessagesHandler::FreeSentMessages()
{
	MessageFactory* messageFactory = MessageFactory::GetInstance();

	while (!_sentMessages.empty())
	{
		Message* message = _sentMessages.front();
		_sentMessages.pop();

		messageFactory->ReleaseMessage(message);
	}
}

void PeerMessagesHandler::ClearMessages()
{
	MessageFactory* messageFactory = MessageFactory::GetInstance();
	assert(messageFactory != nullptr);

	while (!_sentMessages.empty())
	{
		Message* message = _sentMessages.front();
		_sentMessages.pop();

		messageFactory->ReleaseMessage(message);
	}

	for (unsigned int i = 0; i < _pendingMessages.size(); ++i)
	{
		Message* message = _pendingMessages[i];
		_pendingMessages[i] = nullptr;

		messageFactory->ReleaseMessage(message);
	}

	_pendingMessages.clear();
}

PeerMessagesHandler::~PeerMessagesHandler()
{
	ClearMessages();
}
