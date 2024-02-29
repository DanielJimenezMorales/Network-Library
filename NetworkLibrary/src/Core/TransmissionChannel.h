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
	virtual unsigned int GetSizeOfNextUnsentMessage() const = 0;
	void FreeSentMessages();

	virtual void AddReceivedMessage(Message* message) = 0;
	virtual bool ArePendingReadyToProcessMessages() const = 0;
	virtual const Message* GetReadyToProcessMessage() = 0;
	void FreeProcessedMessages();

	virtual void SeUnsentACKsToFalse() = 0;//This method should not exists and be done automatically. However, I have not found how so for now, we do it manually.
	virtual bool AreUnsentACKs() const = 0;
	virtual uint32_t GenerateACKs() const = 0;
	virtual void ProcessACKs(uint32_t acks, uint16_t lastAckedMessageSequenceNumber) = 0;
	virtual bool IsMessageDuplicated(uint16_t messageSequenceNumber) const = 0;

	virtual void Update(float deltaTime) = 0;

	virtual uint16_t GetLastMessageSequenceNumberAcked() const = 0;

	virtual void Reset();

	virtual unsigned int GetRTTMilliseconds() const = 0;

	virtual ~TransmissionChannel();

protected:
	std::vector<Message*> _unsentMessages;
	std::queue<Message*> _sentMessages;
	std::queue<Message*> _readyToProcessMessages;
	std::queue<Message*> _processedMessages;

	virtual void FreeSentMessage(MessageFactory& messageFactory, Message* message) = 0;

	uint16_t GetNextMessageSequenceNumber() const { return _nextMessageSequenceNumber; }
	void IncreaseMessageSequenceNumber() { ++_nextMessageSequenceNumber; };

private:
	TransmissionChannelType _type;
	uint16_t _nextMessageSequenceNumber;

	void ClearMessages();
};

