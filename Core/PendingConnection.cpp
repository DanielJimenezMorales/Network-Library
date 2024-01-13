#include "PendingConnection.h"
#include "Message.h"

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

	return message;
}

PendingConnection::~PendingConnection()
{
}
