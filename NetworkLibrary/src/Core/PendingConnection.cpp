#include "PendingConnection.h"
#include "Message.h"
#include "MessageFactory.h"

PendingConnection::PendingConnection(const Address& addr) : _address(Address(addr.GetInfo())), _clientSalt(0), _serverSalt(0), _messagesHandler()
{
}

bool PendingConnection::AddMessage(Message* message)
{
	if (message == nullptr)
	{
		return false;
	}

	_messagesHandler.AddMessage(message);
	return true;
}

Message* PendingConnection::GetAMessage()
{
	return _messagesHandler.GetAMessage();
}

void PendingConnection::FreeSentMessages()
{
	_messagesHandler.FreeSentMessages();
}

PendingConnection::~PendingConnection()
{
}
