#include "unreliable_unordered_transmission_channel.h"

#include "communication/message_factory.h"

namespace NetLib
{
	UnreliableUnorderedTransmissionChannel::UnreliableUnorderedTransmissionChannel() : TransmissionChannel(TransmissionChannelType::UnreliableUnordered)
	{
	}

	UnreliableUnorderedTransmissionChannel::UnreliableUnorderedTransmissionChannel(UnreliableUnorderedTransmissionChannel&& other) noexcept :
		TransmissionChannel(std::move(other))
	{
	}

	UnreliableUnorderedTransmissionChannel& UnreliableUnorderedTransmissionChannel::operator=(UnreliableUnorderedTransmissionChannel&& other) noexcept
	{
		TransmissionChannel::operator=(std::move(other));
		return *this;
	}

	void UnreliableUnorderedTransmissionChannel::AddMessageToSend(std::unique_ptr<Message> message)
	{
		_unsentMessages.push_back(std::move(message));
	}

	bool UnreliableUnorderedTransmissionChannel::ArePendingMessagesToSend() const
	{
		return (!_unsentMessages.empty());
	}

	std::unique_ptr<Message> UnreliableUnorderedTransmissionChannel::GetMessageToSend()
	{
		if (!ArePendingMessagesToSend())
		{
			return nullptr;
		}

		//TODO Check this. This is not a linked list so if you always get and delete the first element you could not have access to the rest in cse there are more
		std::unique_ptr<Message> message(std::move(_unsentMessages[0]));
		_unsentMessages.erase(_unsentMessages.begin());

		message->SetHeaderPacketSequenceNumber(0);

		return std::move(message);
	}

	uint32 UnreliableUnorderedTransmissionChannel::GetSizeOfNextUnsentMessage() const
	{
		if (!ArePendingMessagesToSend())
		{
			return 0;
		}

		return _unsentMessages.front()->Size();
	}

	void UnreliableUnorderedTransmissionChannel::AddReceivedMessage(std::unique_ptr<Message> message)
	{
		_readyToProcessMessages.push(std::move(message));
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

		std::unique_ptr<Message> message(std::move(_readyToProcessMessages.front()));
		_readyToProcessMessages.pop();

		Message* messageToReturn = message.get();
		_processedMessages.push(std::move(message));

		return messageToReturn;
	}

	void UnreliableUnorderedTransmissionChannel::SeUnsentACKsToFalse()
	{
	}

	bool UnreliableUnorderedTransmissionChannel::AreUnsentACKs() const
	{
		return false;
	}

	uint32 UnreliableUnorderedTransmissionChannel::GenerateACKs() const
	{
		return 0;
	}

	void UnreliableUnorderedTransmissionChannel::ProcessACKs(uint32 acks, uint16 lastAckedMessageSequenceNumber)
	{
	}

	bool UnreliableUnorderedTransmissionChannel::IsMessageDuplicated(uint16 messageSequenceNumber) const
	{
		return false;
	}

	void UnreliableUnorderedTransmissionChannel::Update(float32 deltaTime)
	{
	}

	uint16 UnreliableUnorderedTransmissionChannel::GetLastMessageSequenceNumberAcked() const
	{
		return 0;
	}

	uint32 UnreliableUnorderedTransmissionChannel::GetRTTMilliseconds() const
	{
		return 0;
	}

	UnreliableUnorderedTransmissionChannel::~UnreliableUnorderedTransmissionChannel()
	{
	}

	void UnreliableUnorderedTransmissionChannel::FreeSentMessage(MessageFactory& messageFactory, std::unique_ptr<Message> message)
	{
		messageFactory.ReleaseMessage(std::move(message));
	}
}
