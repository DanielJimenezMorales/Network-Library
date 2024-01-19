#include "PendingConnection.h"
#include "Message.h"
#include "MessageFactory.h"

PendingConnection::PendingConnection(const Address& addr) : _address(Address(addr.GetInfo())), _clientSalt(0), _serverSalt(0)
{
	_pendingMessages.reserve(2);
}

bool PendingConnection::AddMessage(Message* message)
{
	if (message == nullptr)
	{
		return false;
	}

	_pendingMessages.push_back(message);
	return true;
}

Message* PendingConnection::GetAMessage()
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

void PendingConnection::FreeSentMessages()
{
	MessageFactory* messageFactory = MessageFactory::GetInstance();

	while (!_sentMessages.empty())
	{
		Message* message = _sentMessages.front();
		_sentMessages.pop();

		messageFactory->ReleaseMessage(message);
	}
}

PendingConnection::~PendingConnection()
{
}
