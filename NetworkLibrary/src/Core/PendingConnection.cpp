#include "PendingConnection.h"
#include "Message.h"
#include "MessageFactory.h"
#include "UnreliableUnorderedTransmissionChannel.h"
#include "Logger.h"

PendingConnection::PendingConnection(const Address& addr) : _address(Address(addr.GetInfo())), _clientSalt(0), _serverSalt(0)
{
	_transmissionChannel = new UnreliableUnorderedTransmissionChannel();
}

bool PendingConnection::ArePendingMessages() const
{
	return _transmissionChannel->ArePendingMessagesToSend();
}

bool PendingConnection::AddMessage(Message* message)
{
	if (message == nullptr)
	{
		return false;
	}

	_transmissionChannel->AddMessageToSend(message);
	return true;
}

Message* PendingConnection::GetAMessage()
{
	return _transmissionChannel->GetMessageToSend();
}

void PendingConnection::FreeSentMessages()
{
	_transmissionChannel->FreeSentMessages();
}

PendingConnection::~PendingConnection()
{
	delete _transmissionChannel;
	_transmissionChannel = nullptr;
}
