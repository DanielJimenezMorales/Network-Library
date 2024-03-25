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
		unsigned int GetSizeOfNextUnsentMessage() const override;

		void AddReceivedMessage(std::unique_ptr<Message> message) override;
		bool ArePendingReadyToProcessMessages() const override;
		const Message* GetReadyToProcessMessage() override;

		void SeUnsentACKsToFalse() override;
		bool AreUnsentACKs() const override;
		uint32_t GenerateACKs() const override;
		void ProcessACKs(uint32_t acks, uint16_t lastAckedMessageSequenceNumber) override;
		bool IsMessageDuplicated(uint16_t messageSequenceNumber) const override;

		void Update(float deltaTime) override;

		uint16_t GetLastMessageSequenceNumberAcked() const override;
		unsigned int GetRTTMilliseconds() const override;

		~UnreliableUnorderedTransmissionChannel();

	protected:
		void FreeSentMessage(MessageFactory& messageFactory, std::unique_ptr<Message> message) override;

	private:
	};
}
