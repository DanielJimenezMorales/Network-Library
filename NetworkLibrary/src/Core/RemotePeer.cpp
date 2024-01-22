#include <cassert>

#include "RemotePeer.h"
#include "Message.h"
#include "MessageFactory.h"
#include "Logger.h"

RemotePeer::RemotePeer() :
	_address(Address::GetInvalid()),
	_dataPrefix(0),
	_maxInactivityTime(0),
	_inactivityTimeLeft(0),
	_messagesHandler(),
	_nextPacketSequenceNumber(1),
	_lastPacketSequenceNumberAcked(0)
{
}

RemotePeer::RemotePeer(const sockaddr_in& addressInfo, uint16_t id, float maxInactivityTime, uint64_t dataPrefix) :
	_address(Address::GetInvalid()),
	_messagesHandler(),
	_nextPacketSequenceNumber(1),
	_lastPacketSequenceNumberAcked(0)
{
	Connect(addressInfo, id, maxInactivityTime, dataPrefix);
}

RemotePeer::~RemotePeer()
{
	Disconnect();
}

void RemotePeer::Connect(const sockaddr_in& addressInfo, uint16_t id, float maxInactivityTime, uint64_t dataPrefix)
{
	_address = Address(addressInfo);
	_id = id;
	_maxInactivityTime = maxInactivityTime;
	_inactivityTimeLeft = _maxInactivityTime;
	_dataPrefix = dataPrefix;
}

void RemotePeer::Tick(float elapsedTime)
{
	_inactivityTimeLeft -= elapsedTime;

	if (_inactivityTimeLeft < 0.f)
	{
		_inactivityTimeLeft = 0.f;
	}
}

bool RemotePeer::AddMessage(Message* message)
{
	_messagesHandler.AddMessage(message);
	return true;
}

Message* RemotePeer::GetAMessage()
{
	return _messagesHandler.GetAMessage();
}

void RemotePeer::FreeSentMessages()
{
	_messagesHandler.FreeSentMessages();
}

void RemotePeer::Disconnect()
{
	_messagesHandler.ClearMessages();
	_address = Address::GetInvalid();
}
