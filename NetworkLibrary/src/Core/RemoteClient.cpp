#include "RemoteClient.h"
#include "Message.h"
#include "MessageFactory.h"
#include "Logger.h"

RemoteClient::RemoteClient(const sockaddr_in& addressInfo, uint16_t index, float maxInactivityTime, uint64_t dataPrefix) : 
							_index(index), 
							_maxInactivityTime(maxInactivityTime), 
							_inactivityTimeLeft(maxInactivityTime),
							_dataPrefix(dataPrefix)
{
	_address = new Address(addressInfo);
	_pendingMessages.reserve(5);
}

RemoteClient::~RemoteClient()
{
	delete _address;
	_address = nullptr;
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
