#pragma once
#include <memory>

#include "TransmissionChannel.h"
#include "Message.h"

namespace NetLib
{
	class MessageFactory;

	class UnreliableUnorderedTransmissionChannel : public TransmissionChannel
	{
	public:
		UnreliableUnorderedTransmissionChannel();
		UnreliableUnorderedTransmissionChannel(const UnreliableUnorderedTransmissionChannel&) = delete;
		UnreliableUnorderedTransmissionChannel(UnreliableUnorderedTransmissionChannel&& other) noexcept;

		UnreliableUnorderedTransmissionChannel& operator=(const UnreliableUnorderedTransmissionChannel&) = delete;
		UnreliableUnorderedTransmissionChannel& operator=(UnreliableUnorderedTransmissionChannel&& other) noexcept;

		void AddMessageToSend(std::unique_ptr<Message> message) override;
		bool ArePendingMessagesToSend() const override;
		std::unique_ptr<Message> GetMessageToSend() override;
		uint32 GetSizeOfNextUnsentMessage() const override;

		void AddReceivedMessage(std::unique_ptr<Message> message) override;
		bool ArePendingReadyToProcessMessages() const override;
		const Message* GetReadyToProcessMessage() override;

		void SeUnsentACKsToFalse() override;
		bool AreUnsentACKs() const override;
		uint32 GenerateACKs() const override;
		void ProcessACKs(uint32 acks, uint16 lastAckedMessageSequenceNumber) override;
		bool IsMessageDuplicated(uint16 messageSequenceNumber) const override;

		void Update(float32 deltaTime) override;

		uint16 GetLastMessageSequenceNumberAcked() const override;
		uint32 GetRTTMilliseconds() const override;

		~UnreliableUnorderedTransmissionChannel();

	protected:
		void FreeSentMessage(MessageFactory& messageFactory, std::unique_ptr<Message> message) override;

	private:
	};
}
