#include <cassert>

#include "RemotePeer.h"
#include "Message.h"
#include "Logger.h"
#include "UnreliableUnorderedTransmissionChannel.h"
#include "ReliableOrderedChannel.h"

RemotePeer::RemotePeer() :
	_address(Address::GetInvalid()),
	_dataPrefix(0),
	_maxInactivityTime(0),
	_inactivityTimeLeft(0),
	_messagesHandler(),
	_nextPacketSequenceNumber(0)
{
	TransmissionChannel* unreliableUnordered = new UnreliableUnorderedTransmissionChannel();
	TransmissionChannel* reliableOrdered = new ReliableOrderedChannel();

	_transmissionChannels[unreliableUnordered->GetType()] = unreliableUnordered;
	_transmissionChannels[reliableOrdered->GetType()] = reliableOrdered;
}

RemotePeer::RemotePeer(const sockaddr_in& addressInfo, uint16_t id, float maxInactivityTime, uint64_t dataPrefix) :
	_address(Address::GetInvalid()),
	_messagesHandler(),
	_nextPacketSequenceNumber(0)
{
	TransmissionChannel* unreliableUnordered = new UnreliableUnorderedTransmissionChannel();
	TransmissionChannel* reliableOrdered = new ReliableOrderedChannel();

	_transmissionChannels[unreliableUnordered->GetType()] = unreliableUnordered;
	_transmissionChannels[reliableOrdered->GetType()] = reliableOrdered;

	Connect(addressInfo, id, maxInactivityTime, dataPrefix);
}

RemotePeer::~RemotePeer()
{
	Disconnect();

	//TODO Free memory from transmission channels map
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

	//Quitar lo de arriba y dejar esto
	TransmissionChannelType transmissionChannelType = GetTransmissionChannelTypeFromHeader(message->GetHeader());
	std::map<TransmissionChannelType, TransmissionChannel*>::iterator it = _transmissionChannels.find(transmissionChannelType);
	if (it != _transmissionChannels.end())
	{
		(*it).second->AddMessageToSend(message);
		return true;
	}
	else
	{
		//En este caso ver qué hacer con el mensaje que nos pasan por parámetro
		return false;
	}
}

TransmissionChannelType RemotePeer::GetTransmissionChannelTypeFromHeader(const MessageHeader& messageHeader) const
{
	TransmissionChannelType result;

	if (messageHeader.isReliable)
	{
		result = TransmissionChannelType::ReliableOrdered;
	}
	else
	{
		result = TransmissionChannelType::UnreliableUnordered;
	}

	return result;
}

Message* RemotePeer::GetPendingMessage()
{
	return _messagesHandler.GetPendingMessage();
}

Message* RemotePeer::GetPendingACKReliableMessage()
{
	return _messagesHandler.GetPendingACKReliableMessage();
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
