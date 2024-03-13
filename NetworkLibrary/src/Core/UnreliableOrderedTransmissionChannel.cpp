#include <memory>

#include "UnreliableOrderedTransmissionChannel.h"
#include "MessageFactory.h"
#include "Logger.h"

UnreliableOrderedTransmissionChannel::UnreliableOrderedTransmissionChannel() :
	TransmissionChannel(TransmissionChannelType::UnreliableUnordered),
	_lastMessageSequenceNumberReceived(0)
{
}

UnreliableOrderedTransmissionChannel::UnreliableOrderedTransmissionChannel(UnreliableOrderedTransmissionChannel&& other) noexcept :
	TransmissionChannel(std::move(other)),
	_lastMessageSequenceNumberReceived(std::move(other._lastMessageSequenceNumberReceived)) //unnecessary move, just in case I change that type
{
}

UnreliableOrderedTransmissionChannel& UnreliableOrderedTransmissionChannel::operator=(UnreliableOrderedTransmissionChannel&& other) noexcept
{
	_lastMessageSequenceNumberReceived = std::move(other._lastMessageSequenceNumberReceived); //unnecessary move, just in case I change that type

	TransmissionChannel::operator=(std::move(other));
	return *this;
}

void UnreliableOrderedTransmissionChannel::AddMessageToSend(std::unique_ptr<Message> message)
{
	_unsentMessages.push_back(std::move(message));
}

bool UnreliableOrderedTransmissionChannel::ArePendingMessagesToSend() const
{
	return (!_unsentMessages.empty());
}

Message* UnreliableOrderedTransmissionChannel::GetMessageToSend()
{
	if (!ArePendingMessagesToSend())
	{
		return nullptr;
	}

	std::unique_ptr<Message> message(std::move(_unsentMessages[0]));
	_unsentMessages.erase(_unsentMessages.begin());

	uint16_t sequenceNumber = GetNextMessageSequenceNumber();
	IncreaseMessageSequenceNumber();

	message->SetHeaderPacketSequenceNumber(sequenceNumber);

	Message* messageToReturn = message.get();
	_sentMessages.push(std::move(message));

	return messageToReturn;
}

unsigned int UnreliableOrderedTransmissionChannel::GetSizeOfNextUnsentMessage() const
{
	if (!ArePendingMessagesToSend())
	{
		return 0;
	}

	return _unsentMessages.front()->Size();
}

void UnreliableOrderedTransmissionChannel::AddReceivedMessage(std::unique_ptr<Message> message)
{
	if (!IsSequenceNumberNewerThanLastReceived(message->GetHeader().messageSequenceNumber))
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		messageFactory.ReleaseMessage(std::move(message));
		return;
	}

	_lastMessageSequenceNumberReceived = message->GetHeader().messageSequenceNumber;
	_readyToProcessMessages.push(std::move(message));
}

bool UnreliableOrderedTransmissionChannel::ArePendingReadyToProcessMessages() const
{
	return (!_readyToProcessMessages.empty());
}

const Message* UnreliableOrderedTransmissionChannel::GetReadyToProcessMessage()
{
	if (!ArePendingReadyToProcessMessages())
	{
		return nullptr;
	}

	std::unique_ptr<Message> message(std::move(_readyToProcessMessages.front()));
	_readyToProcessMessages.pop();

	Message* messageToReturn = message.get();
	_processedMessages.push(std::move(message));

	return messageToReturn;
}

void UnreliableOrderedTransmissionChannel::SeUnsentACKsToFalse()
{
}

bool UnreliableOrderedTransmissionChannel::AreUnsentACKs() const
{
	return false;
}

uint32_t UnreliableOrderedTransmissionChannel::GenerateACKs() const
{
	return 0;
}

void UnreliableOrderedTransmissionChannel::ProcessACKs(uint32_t acks, uint16_t lastAckedMessageSequenceNumber)
{
	//This channel is not supporting ACKs since it is unreliable. So do nothing
}

bool UnreliableOrderedTransmissionChannel::IsMessageDuplicated(uint16_t messageSequenceNumber) const
{
	return false;
}

void UnreliableOrderedTransmissionChannel::Update(float deltaTime)
{
}

uint16_t UnreliableOrderedTransmissionChannel::GetLastMessageSequenceNumberAcked() const
{
	return 0;
}

unsigned int UnreliableOrderedTransmissionChannel::GetRTTMilliseconds() const
{
	return 0;
}

UnreliableOrderedTransmissionChannel::~UnreliableOrderedTransmissionChannel()
{
}

void UnreliableOrderedTransmissionChannel::FreeSentMessage(MessageFactory& messageFactory, std::unique_ptr<Message> message)
{
	messageFactory.ReleaseMessage(std::move(message));
}

bool UnreliableOrderedTransmissionChannel::IsSequenceNumberNewerThanLastReceived(uint32_t sequenceNumber) const
{
	//The second part of the if is to support the case when sequence number reaches its limit value and wraps around
	if (sequenceNumber > _lastMessageSequenceNumberReceived || (_lastMessageSequenceNumberReceived - sequenceNumber) >= UINT32_HALF)
	{
		return true;
	}
	else
	{
		return false;
	}
}
