#pragma once
#include <queue>
#include <vector>

class Message;
class MessageFactory;

enum TransmissionChannelType : uint8_t
{
	UnreliableUnordered = 0,
	ReliableOrdered = 1
};

class TransmissionChannel
{
public:
	TransmissionChannel(TransmissionChannelType type);

	TransmissionChannelType GetType() { return _type; }

	virtual void AddMessageToSend(Message* message) = 0;
	virtual bool ArePendingMessagesToSend() const = 0;
	virtual Message* GetMessageToSend() = 0;
	unsigned int GetSizeOfNextUnsentMessage() const;
	void FreeSentMessages();

	virtual void AddReceivedMessage(Message* message) = 0;
	virtual bool ArePendingReadyToProcessMessages() const = 0;
	virtual const Message* GetReadyToProcessMessage() = 0;
	void FreeProcessedMessages();

	virtual uint32_t GenerateACKs() const = 0;
	virtual void ProcessACKs(uint32_t acks, uint16_t lastAckedMessageSequenceNumber) = 0;
	virtual bool IsMessageDuplicated(uint16_t messageSequenceNumber) const = 0;

	virtual void Update(float deltaTime) = 0;

	virtual ~TransmissionChannel();

protected:
	std::vector<Message*> _unsentMessages;
	std::queue<Message*> _sentMessages;
	std::queue<Message*> _readyToProcessMessages;
	std::queue<Message*> _processedMessages;

	virtual void FreeSentMessage(MessageFactory& messageFactory, Message* message) = 0;

private:
	TransmissionChannelType _type;

	void ClearMessages();
};

