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

	//Free transmission channel memory
	std::map<TransmissionChannelType, TransmissionChannel*>::iterator it = _transmissionChannels.begin();
	while (it != _transmissionChannels.end())
	{
		LOG_INFO("destructor");
		TransmissionChannel* transmissionChannel = it->second;
		delete transmissionChannel;

		++it;
	}

	_transmissionChannels.clear();
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

	//Update transmission channels
	std::map<TransmissionChannelType, TransmissionChannel*>::iterator it = _transmissionChannels.begin();
	while (it != _transmissionChannels.end())
	{
		it->second->Update(elapsedTime);
		++it;
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
		it->second->AddMessageToSend(message);
		return true;
	}
	else
	{
		//En este caso ver qu� hacer con el mensaje que nos pasan por par�metro
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

	//Quitar lo de arriba despu�s
	std::map<TransmissionChannelType, TransmissionChannel*>::iterator it = _transmissionChannels.begin();

	while (it != _transmissionChannels.end())
	{
		it->second->FreeSentMessages();

		++it;
	}
}

void RemotePeer::FreeProcessedMessages()
{
	_messagesHandler.FreeProcessedMessages();

	//Quitar lo de arriba despu�s
	std::map<TransmissionChannelType, TransmissionChannel*>::iterator it = _transmissionChannels.begin();

	while (it != _transmissionChannels.end())
	{
		it->second->FreeProcessedMessages();

		++it;
	}
}

bool RemotePeer::AddReceivedMessage(Message* message)
{
	return _messagesHandler.AddReceivedMessage(message);

	//Quitar lo de arriba despu�s
	TransmissionChannelType channelType = GetTransmissionChannelTypeFromHeader(message->GetHeader());
	std::map<TransmissionChannelType, TransmissionChannel*>::iterator it = _transmissionChannels.find(channelType);
	if (it != _transmissionChannels.end())
	{
		it->second->AddReceivedMessage(message);
		return true;
	}
	else
	{
		//En este caso ver qu� hacer con el mensaje que nos pasan por par�metro
		return false;
	}
}

bool RemotePeer::ArePendingReadyToProcessMessages() const
{
	return _messagesHandler.ArePendingReadyToProcessMessages();

	//Quitar lo de arriba despu�s
	bool areReadyToProcessMessages = false;

	std::map<TransmissionChannelType, TransmissionChannel*>::const_iterator cit = _transmissionChannels.cbegin();

	while (cit != _transmissionChannels.cend())
	{
		if (cit->second->ArePendingReadyToProcessMessages())
		{
			areReadyToProcessMessages = true;
			break;
		}

		++cit;
	}

	return areReadyToProcessMessages;
}

const Message* RemotePeer::GetPendingReadyToProcessMessage()
{
	return _messagesHandler.GetReadyToProcessMessage();

	//Quitar lo de arriba despu�s
	const Message* message = nullptr;
	std::map<TransmissionChannelType, TransmissionChannel*>::iterator it = _transmissionChannels.begin();

	while (it != _transmissionChannels.end())
	{
		if (it->second->ArePendingReadyToProcessMessages())
		{
			message = it->second->GetReadyToProcessMessage();
			break;
		}

		++it;
	}

	return message;
}

void RemotePeer::Disconnect()
{
	_messagesHandler.ClearMessages();
	_address = Address::GetInvalid();
}
