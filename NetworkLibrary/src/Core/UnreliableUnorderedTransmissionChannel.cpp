#include "UnreliableUnorderedTransmissionChannel.h"
#include "MessageFactory.h"

UnreliableUnorderedTransmissionChannel::UnreliableUnorderedTransmissionChannel() : TransmissionChannel(TransmissionChannelType::UnreliableUnordered)
{
}

void UnreliableUnorderedTransmissionChannel::AddMessageToSend(Message* message)
{
	_unsentMessages.push_back(message);
}

bool UnreliableUnorderedTransmissionChannel::ArePendingMessagesToSend() const
{
	return (!_unsentMessages.empty());
}

Message* UnreliableUnorderedTransmissionChannel::GetMessageToSend()
{
	if (!ArePendingMessagesToSend())
	{
		return nullptr;
	}

	Message* message = _unsentMessages[0];
	_unsentMessages.erase(_unsentMessages.begin());

	_sentMessages.push(message);
	return message;
}

unsigned int UnreliableUnorderedTransmissionChannel::GetSizeOfNextUnsentMessage() const
{
	if (!ArePendingMessagesToSend())
	{
		return 0;
	}

	return _unsentMessages.front()->Size();
}

void UnreliableUnorderedTransmissionChannel::AddReceivedMessage(Message* message)
{
	_readyToProcessMessages.push(message);
}

bool UnreliableUnorderedTransmissionChannel::ArePendingReadyToProcessMessages() const
{
	return (!_readyToProcessMessages.empty());
}

const Message* UnreliableUnorderedTransmissionChannel::GetReadyToProcessMessage()
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

void UnreliableUnorderedTransmissionChannel::SeUnsentACKsToFalse()
{
}

bool UnreliableUnorderedTransmissionChannel::AreUnsentACKs() const
{
	return false;
}

uint32_t UnreliableUnorderedTransmissionChannel::GenerateACKs() const
{
	return 0;
}

void UnreliableUnorderedTransmissionChannel::ProcessACKs(uint32_t acks, uint16_t lastAckedMessageSequenceNumber)
{
	//This channel is not supporting ACKs since it is unreliable. So do nothing
}

bool UnreliableUnorderedTransmissionChannel::IsMessageDuplicated(uint16_t messageSequenceNumber) const
{
	return false;
}

void UnreliableUnorderedTransmissionChannel::Update(float deltaTime)
{
}

uint16_t UnreliableUnorderedTransmissionChannel::GetLastMessageSequenceNumberAcked() const
{
	return 0;
}

UnreliableUnorderedTransmissionChannel::~UnreliableUnorderedTransmissionChannel()
{
}

void UnreliableUnorderedTransmissionChannel::FreeSentMessage(MessageFactory& messageFactory, Message* message)
{
	messageFactory.ReleaseMessage(message);
}
