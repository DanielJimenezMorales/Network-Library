#include <cassert>

#include "RemoteClient.h"
#include "Message.h"
#include "MessageFactory.h"
#include "Logger.h"

RemoteClient::RemoteClient() : _address(Address::GetInvalid()), _dataPrefix(0), _maxInactivityTime(0), _inactivityTimeLeft(0), _messagesHandler()
{
}

RemoteClient::RemoteClient(const sockaddr_in& addressInfo, uint16_t id, float maxInactivityTime, uint64_t dataPrefix) : _address(Address::GetInvalid()), _messagesHandler()
{
	Connect(addressInfo, id, maxInactivityTime, dataPrefix);
}

RemoteClient::~RemoteClient()
{
	Disconnect();
}

void RemoteClient::Connect(const sockaddr_in& addressInfo, uint16_t id, float maxInactivityTime, uint64_t dataPrefix)
{
	_address = Address(addressInfo);
	_id = id;
	_maxInactivityTime = maxInactivityTime;
	_inactivityTimeLeft = _maxInactivityTime;
	_dataPrefix = dataPrefix;
}

void RemoteClient::Tick(float elapsedTime)
{
	_inactivityTimeLeft -= elapsedTime;

	if (_inactivityTimeLeft < 0.f)
	{
		_inactivityTimeLeft = 0.f;
	}
}

bool RemoteClient::AddMessage(Message* message)
{
	_messagesHandler.AddMessage(message);
	return true;
}

Message* RemoteClient::GetAMessage()
{
	return _messagesHandler.GetAMessage();
}

void RemoteClient::FreeSentMessages()
{
	_messagesHandler.FreeSentMessages();
}

void RemoteClient::Disconnect()
{
	_messagesHandler.ClearMessages();
	_address = Address::GetInvalid();
}
