#include <memory>

#include "UnreliableOrderedTransmissionChannel.h"
#include "MessageFactory.h"
#include "Logger.h"

UnreliableOrderedTransmissionChannel::UnreliableOrderedTransmissionChannel() : _lastMessageSequenceNumberReceived(0), TransmissionChannel(TransmissionChannelType::UnreliableUnordered)
{
}

void UnreliableOrderedTransmissionChannel::AddMessageToSend(Message* message)
{
	_unsentMessages.push_back(message);
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

	Message* message = _unsentMessages[0];
	_unsentMessages.erase(_unsentMessages.begin());

	uint16_t sequenceNumber = GetNextMessageSequenceNumber();
	IncreaseMessageSequenceNumber();

	message->SetHeaderPacketSequenceNumber(sequenceNumber);

	_sentMessages.push(message);
	return message;
}

unsigned int UnreliableOrderedTransmissionChannel::GetSizeOfNextUnsentMessage() const
{
	if (!ArePendingMessagesToSend())
	{
		return 0;
	}

	return _unsentMessages.front()->Size();
}

void UnreliableOrderedTransmissionChannel::AddReceivedMessage(Message* message)
{
	std::unique_ptr<Message> messageHandler(message);
	if (!IsSequenceNumberNewerThanLastReceived(message->GetHeader().messageSequenceNumber))
	{
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		messageFactory.ReleaseMessage(std::move(messageHandler));
		return;
	}

	_lastMessageSequenceNumberReceived = message->GetHeader().messageSequenceNumber;
	_readyToProcessMessages.push(messageHandler.release());
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

	Message* message = nullptr;
	message = _readyToProcessMessages.front();
	_readyToProcessMessages.pop();

	_processedMessages.push(message);
	return message;
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

void UnreliableOrderedTransmissionChannel::FreeSentMessage(MessageFactory& messageFactory, Message* message)
{
	std::unique_ptr<Message> messageHandler(message);
	messageFactory.ReleaseMessage(std::move(messageHandler));
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
