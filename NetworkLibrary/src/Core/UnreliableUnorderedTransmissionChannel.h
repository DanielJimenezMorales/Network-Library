#pragma once
#include "TransmissionChannel.h"

class UnreliableUnorderedTransmissionChannel : public TransmissionChannel
{
public:
	UnreliableUnorderedTransmissionChannel();

	void AddMessageToSend(Message* message) override;
	bool ArePendingMessagesToSend() const override;
	Message* GetMessageToSend() override;

	void AddReceivedMessage(Message* message) override;
	bool ArePendingReadyToProcessMessages() const override;
	const Message* GetReadyToProcessMessage() override;

	uint32_t GenerateACKs() const override;
	void ProcessACKs(uint32_t acks, uint16_t lastAckedMessageSequenceNumber) override;
	bool IsMessageDuplicated(uint16_t messageSequenceNumber) const override;

protected:
	void FreeSentMessage(MessageFactory& messageFactory, Message* message) override;
};

