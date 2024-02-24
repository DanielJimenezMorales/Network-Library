#include <cassert>
#include <sstream>

#include "RemotePeer.h"
#include "Message.h"
#include "Logger.h"
#include "UnreliableUnorderedTransmissionChannel.h"
#include "ReliableOrderedChannel.h"

TransmissionChannel* RemotePeer::GetTransmissionChannelFromType(TransmissionChannelType channelType)
{
	TransmissionChannel* transmissionChannel = nullptr;

	std::map<TransmissionChannelType, TransmissionChannel*>::iterator it = _transmissionChannels.find(channelType);
	if (it != _transmissionChannels.end())
	{
		transmissionChannel = it->second;
	}

	return transmissionChannel;
}

const TransmissionChannel* RemotePeer::GetTransmissionChannelFromType(TransmissionChannelType channelType) const
{
	const TransmissionChannel* transmissionChannel = nullptr;

	std::map<TransmissionChannelType, TransmissionChannel*>::const_iterator cit = _transmissionChannels.find(channelType);
	if (cit != _transmissionChannels.cend())
	{
		transmissionChannel = cit->second;
	}

	return transmissionChannel;
}

RemotePeer::RemotePeer() :
	_address(Address::GetInvalid()),
	_dataPrefix(0),
	_maxInactivityTime(0),
	_inactivityTimeLeft(0),
	_nextPacketSequenceNumber(0)
{
	TransmissionChannel* unreliableUnordered = new UnreliableUnorderedTransmissionChannel();
	TransmissionChannel* reliableOrdered = new ReliableOrderedChannel();

	_transmissionChannels[unreliableUnordered->GetType()] = unreliableUnordered;
	_transmissionChannels[reliableOrdered->GetType()] = reliableOrdered;
}

RemotePeer::RemotePeer(const sockaddr_in& addressInfo, uint16_t id, float maxInactivityTime, uint64_t dataPrefix) :
	_address(Address::GetInvalid()),
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

uint16_t RemotePeer::GetLastMessageSequenceNumberAcked(TransmissionChannelType channelType) const
{
	uint16_t lastMessageSequenceNumberAcked = 0;

	const TransmissionChannel* transmissionChannel = GetTransmissionChannelFromType(channelType);
	if (transmissionChannel != nullptr)
	{
		lastMessageSequenceNumberAcked = transmissionChannel->GetLastMessageSequenceNumberAcked();
	}

	return lastMessageSequenceNumberAcked;
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
	TransmissionChannelType channelType = GetTransmissionChannelTypeFromHeader(message->GetHeader());

	TransmissionChannel* transmissionChannel = GetTransmissionChannelFromType(channelType);
	if (transmissionChannel != nullptr)
	{
		transmissionChannel->AddMessageToSend(message); return true;
	}
	else
	{
		//TODO En este caso ver qué hacer con el mensaje que nos pasan por parámetro
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

bool RemotePeer::ArePendingMessages(TransmissionChannelType channelType) const
{
	bool arePendingMessages = false;

	const TransmissionChannel* transmissionChannel = GetTransmissionChannelFromType(channelType);
	if (transmissionChannel != nullptr)
	{
		arePendingMessages = transmissionChannel->ArePendingMessagesToSend();
	}

	return arePendingMessages;
}

Message* RemotePeer::GetPendingMessage(TransmissionChannelType channelType)
{
	Message* message = nullptr;

	TransmissionChannel* transmissionChannel = GetTransmissionChannelFromType(channelType);
	if (transmissionChannel != nullptr)
	{
		message = transmissionChannel->GetMessageToSend();
	}

	return message;
}

unsigned int RemotePeer::GetSizeOfNextUnsentMessage(TransmissionChannelType channelType) const
{
	unsigned int size = 0;

	const TransmissionChannel* transmissionChannel = GetTransmissionChannelFromType(channelType);
	if (transmissionChannel != nullptr)
	{
		size = transmissionChannel->GetSizeOfNextUnsentMessage();
	}

	return size;
}

void RemotePeer::FreeSentMessages()
{
	std::map<TransmissionChannelType, TransmissionChannel*>::iterator it = _transmissionChannels.begin();
	while (it != _transmissionChannels.end())
	{
		it->second->FreeSentMessages();

		++it;
	}
}

void RemotePeer::FreeProcessedMessages()
{
	std::map<TransmissionChannelType, TransmissionChannel*>::iterator it = _transmissionChannels.begin();
	while (it != _transmissionChannels.end())
	{
		it->second->FreeProcessedMessages();

		++it;
	}
}

void RemotePeer::SeUnsentACKsToFalse(TransmissionChannelType channelType)
{
	TransmissionChannel* transmissionChannel = GetTransmissionChannelFromType(channelType);
	if (transmissionChannel != nullptr)
	{
		transmissionChannel->SeUnsentACKsToFalse();
	}
}

bool RemotePeer::AreUnsentACKs(TransmissionChannelType channelType) const
{
	bool areUnsentACKs = false;

	const TransmissionChannel* transmissionChannel = GetTransmissionChannelFromType(channelType);
	if (transmissionChannel != nullptr)
	{
		areUnsentACKs = transmissionChannel->AreUnsentACKs();
	}

	return areUnsentACKs;
}

uint32_t RemotePeer::GenerateACKs(TransmissionChannelType channelType) const
{
	uint32_t acks = 0;

	const TransmissionChannel* transmissionChannel = GetTransmissionChannelFromType(channelType);
	if (transmissionChannel != nullptr)
	{
		acks = transmissionChannel->GenerateACKs();
	}

	return acks;
}

void RemotePeer::ProcessACKs(uint32_t acks, uint16_t lastAckedMessageSequenceNumber, TransmissionChannelType channelType)
{
	TransmissionChannel* transmissionChannel = GetTransmissionChannelFromType(channelType);
	if (transmissionChannel != nullptr)
	{
		transmissionChannel->ProcessACKs(acks, lastAckedMessageSequenceNumber);
	}
}

bool RemotePeer::AddReceivedMessage(Message* message)
{
	TransmissionChannelType channelType = GetTransmissionChannelTypeFromHeader(message->GetHeader());

	TransmissionChannel* transmissionChannel = GetTransmissionChannelFromType(channelType);
	if (transmissionChannel != nullptr)
	{
		transmissionChannel->AddReceivedMessage(message);
		return true;
	}
	else
	{
		//TODO En este caso ver qué hacer con el mensaje que nos pasan por parámetro
		return false;
	}
}

bool RemotePeer::ArePendingReadyToProcessMessages() const
{
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

bool RemotePeer::GetNumberOfTransmissionChannels() const
{
	return _transmissionChannels.size();
}

void RemotePeer::Disconnect()
{
	//Reset transmission channels
	std::map<TransmissionChannelType, TransmissionChannel*>::iterator it = _transmissionChannels.begin();
	while (it != _transmissionChannels.end())
	{
		it->second->Reset();
		++it;
	}

	//Reset address
	_address = Address::GetInvalid();
}
