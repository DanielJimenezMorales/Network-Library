#include <cassert>
#include <sstream>
#include <memory>

#include "RemotePeer.h"
#include "Message.h"
#include "Logger.h"
#include "UnreliableOrderedTransmissionChannel.h"
#include "UnreliableUnorderedTransmissionChannel.h"
#include "ReliableOrderedChannel.h"

namespace NetLib
{
	void RemotePeer::InitTransmissionChannels()
	{
		TransmissionChannel* unreliableOrdered = new UnreliableOrderedTransmissionChannel();
		TransmissionChannel* unreliableUnordered = new UnreliableUnorderedTransmissionChannel();
		TransmissionChannel* reliableOrdered = new ReliableOrderedChannel();

		_transmissionChannels.push_back(unreliableOrdered);
		_transmissionChannels.push_back(unreliableUnordered);
		_transmissionChannels.push_back(reliableOrdered);
	}

	TransmissionChannel* RemotePeer::GetTransmissionChannelFromType(TransmissionChannelType channelType)
	{
		TransmissionChannel* transmissionChannel = nullptr;

		for (unsigned int i = 0; i < GetNumberOfTransmissionChannels(); ++i)
		{
			if (_transmissionChannels[i]->GetType() == channelType)
			{
				transmissionChannel = _transmissionChannels[i];
				break;
			}
		}

		return transmissionChannel;
	}

	const TransmissionChannel* RemotePeer::GetTransmissionChannelFromType(TransmissionChannelType channelType) const
	{
		const TransmissionChannel* transmissionChannel = nullptr;

		for (unsigned int i = 0; i < GetNumberOfTransmissionChannels(); ++i)
		{
			if (_transmissionChannels[i]->GetType() == channelType)
			{
				transmissionChannel = _transmissionChannels[i];
				break;
			}
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
		InitTransmissionChannels();
	}

	RemotePeer::RemotePeer(const sockaddr_in& addressInfo, uint16_t id, float maxInactivityTime, uint64_t dataPrefix) :
		_address(Address::GetInvalid()),
		_nextPacketSequenceNumber(0)
	{
		InitTransmissionChannels();
		Connect(addressInfo, id, maxInactivityTime, dataPrefix);
	}

	RemotePeer::~RemotePeer()
	{
		Disconnect();

		//Free transmission channel memory
		for (unsigned int i = 0; i < _transmissionChannels.size(); ++i)
		{
			LOG_INFO("destructor");
			delete _transmissionChannels[i];
			_transmissionChannels[i] = nullptr;
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
		for (unsigned int i = 0; i < GetNumberOfTransmissionChannels(); ++i)
		{
			_transmissionChannels[i]->Update(elapsedTime);
		}
	}

	bool RemotePeer::AddMessage(std::unique_ptr<Message> message)
	{
		TransmissionChannelType channelType = GetTransmissionChannelTypeFromHeader(message->GetHeader());

		TransmissionChannel* transmissionChannel = GetTransmissionChannelFromType(channelType);
		if (transmissionChannel != nullptr)
		{
			transmissionChannel->AddMessageToSend(std::move(message));
			return true;
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

		if (messageHeader.isReliable && messageHeader.isOrdered)
		{
			result = TransmissionChannelType::ReliableOrdered;
		}
		else if (!messageHeader.isReliable && messageHeader.isOrdered)
		{
			result = TransmissionChannelType::UnreliableOrdered;
		}
		else if (!messageHeader.isReliable && !messageHeader.isOrdered)
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

	std::unique_ptr<Message> RemotePeer::GetPendingMessage(TransmissionChannelType channelType)
	{
		std::unique_ptr<Message> message = nullptr;

		TransmissionChannel* transmissionChannel = GetTransmissionChannelFromType(channelType);
		if (transmissionChannel != nullptr)
		{
			message = transmissionChannel->GetMessageToSend();
		}

		return std::move(message);
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
		for (unsigned int i = 0; i < GetNumberOfTransmissionChannels(); ++i)
		{
			_transmissionChannels[i]->FreeSentMessages();
		}
	}

	void RemotePeer::AddSentMessage(std::unique_ptr<Message> message, TransmissionChannelType channelType)
	{
		TransmissionChannel* transmissionChannel = GetTransmissionChannelFromType(channelType);
		if (transmissionChannel != nullptr)
		{
			transmissionChannel->AddSentMessage(std::move(message));
		}
	}

	void RemotePeer::FreeProcessedMessages()
	{
		for (unsigned int i = 0; i < GetNumberOfTransmissionChannels(); ++i)
		{
			_transmissionChannels[i]->FreeProcessedMessages();
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

	bool RemotePeer::AddReceivedMessage(std::unique_ptr<Message> message)
	{
		TransmissionChannelType channelType = GetTransmissionChannelTypeFromHeader(message->GetHeader());

		TransmissionChannel* transmissionChannel = GetTransmissionChannelFromType(channelType);
		if (transmissionChannel != nullptr)
		{
			transmissionChannel->AddReceivedMessage(std::move(message));
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

		for (unsigned int i = 0; i < GetNumberOfTransmissionChannels(); ++i)
		{
			if (_transmissionChannels[i]->ArePendingReadyToProcessMessages())
			{
				areReadyToProcessMessages = true;
				break;
			}
		}

		return areReadyToProcessMessages;
	}

	const Message* RemotePeer::GetPendingReadyToProcessMessage()
	{
		const Message* message = nullptr;

		for (unsigned int i = 0; i < GetNumberOfTransmissionChannels(); ++i)
		{
			if (_transmissionChannels[i]->ArePendingReadyToProcessMessages())
			{
				message = _transmissionChannels[i]->GetReadyToProcessMessage();
				break;
			}
		}

		return message;
	}

	std::vector<TransmissionChannelType> RemotePeer::GetAvailableTransmissionChannelTypes() const
	{
		std::vector<TransmissionChannelType> channelTypes;
		channelTypes.reserve(GetNumberOfTransmissionChannels());

		std::vector<TransmissionChannel*>::const_iterator cit = _transmissionChannels.cbegin();
		for (unsigned int i = 0; i < GetNumberOfTransmissionChannels(); ++i)
		{
			channelTypes.push_back((*(cit + i))->GetType());
		}

		return channelTypes;
	}

	unsigned int RemotePeer::GetNumberOfTransmissionChannels() const
	{
		return _transmissionChannels.size();
	}

	unsigned int RemotePeer::GetRTTMilliseconds() const
	{
		unsigned int rtt = 0;
		unsigned int numberOfTransmissionChannels = 0;

		for (unsigned int i = 0; i < numberOfTransmissionChannels; ++i)
		{
			unsigned int transmissionChannelRTT = _transmissionChannels[i]->GetRTTMilliseconds();
			if (transmissionChannelRTT > 0)
			{
				rtt += transmissionChannelRTT;
				++numberOfTransmissionChannels;
			}
		}

		if (numberOfTransmissionChannels > 0)
		{
			rtt /= numberOfTransmissionChannels;
		}

		return rtt;
	}

	void RemotePeer::Disconnect()
	{
		//Reset transmission channels
		for (unsigned int i = 0; i < GetNumberOfTransmissionChannels(); ++i)
		{
			_transmissionChannels[i]->Reset();
		}

		//Reset address
		_address = Address::GetInvalid();
	}
}
