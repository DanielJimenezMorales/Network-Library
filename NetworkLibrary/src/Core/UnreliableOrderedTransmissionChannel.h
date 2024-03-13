#pragma once
#include "TransmissionChannel.h"

#define UINT32_HALF 2147483647

class UnreliableOrderedTransmissionChannel : public TransmissionChannel
{
public:
	UnreliableOrderedTransmissionChannel();
	UnreliableOrderedTransmissionChannel(const UnreliableOrderedTransmissionChannel&) = delete;
	UnreliableOrderedTransmissionChannel(UnreliableOrderedTransmissionChannel&& other) noexcept;

	UnreliableOrderedTransmissionChannel& operator=(const UnreliableOrderedTransmissionChannel&) = delete;
	UnreliableOrderedTransmissionChannel& operator=(UnreliableOrderedTransmissionChannel&& other) noexcept;

	void AddMessageToSend(std::unique_ptr<Message> message) override;
	bool ArePendingMessagesToSend() const override;
	Message* GetMessageToSend() override;
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

	~UnreliableOrderedTransmissionChannel();

protected:
	void FreeSentMessage(MessageFactory& messageFactory, std::unique_ptr<Message> message) override;

private:
	uint32_t _lastMessageSequenceNumberReceived;

	bool IsSequenceNumberNewerThanLastReceived(uint32_t sequenceNumber) const;
};

