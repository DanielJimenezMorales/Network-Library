#include <cassert>
#include "RemoteClient.h"
#include "Message.h"
#include "MessageFactory.h"
#include "Logger.h"

RemoteClient::RemoteClient() : _address(Address::GetInvalid()), _dataPrefix(0), _maxInactivityTime(0), _inactivityTimeLeft(0)
{
	_pendingMessages.reserve(5);
}

RemoteClient::RemoteClient(const sockaddr_in& addressInfo, uint16_t id, float maxInactivityTime, uint64_t dataPrefix) : _address(Address::GetInvalid())
{
	_pendingMessages.reserve(5);

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
	_pendingMessages.push_back(message);
	return true;
}

Message* RemoteClient::GetAMessage()
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

void RemoteClient::FreeSentMessages()
{
	MessageFactory* messageFactory = MessageFactory::GetInstance();

	while (!_sentMessages.empty())
	{
		Message* message = _sentMessages.front();
		_sentMessages.pop();
		messageFactory->ReleaseMessage(message);
	}
}

void RemoteClient::Disconnect()
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

	_address = Address::GetInvalid();
}
