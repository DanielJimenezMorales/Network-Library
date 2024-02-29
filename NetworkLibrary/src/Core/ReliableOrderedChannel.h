#pragma once
#include <list>
#include <unordered_map>

#include "TransmissionChannel.h"
#include "PeerMessagesHandler.h"

class Message;

class ReliableOrderedChannel : public TransmissionChannel
{
public:
	ReliableOrderedChannel();

	void AddMessageToSend(Message* message) override;
	bool ArePendingMessagesToSend() const override;
	Message* GetMessageToSend() override;
	unsigned int GetSizeOfNextUnsentMessage() const override;

	void AddReceivedMessage(Message* message) override;
	bool ArePendingReadyToProcessMessages() const override;
	const Message* GetReadyToProcessMessage() override;

	void SeUnsentACKsToFalse() override;
	bool AreUnsentACKs() const override;
	uint32_t GenerateACKs() const override;
	void ProcessACKs(uint32_t acks, uint16_t lastAckedMessageSequenceNumber) override;
	bool IsMessageDuplicated(uint16_t messageSequenceNumber) const override;

	void Update(float deltaTime) override;

	uint16_t GetLastMessageSequenceNumberAcked() const override;

	void Reset() override;

	unsigned int GetRTTMilliseconds() const override;

	~ReliableOrderedChannel();

protected:
	void FreeSentMessage(MessageFactory& messageFactory, Message* message) override;

private:
	std::list<Message*> _unackedReliableMessages;
	std::list<float> _unackedReliableMessageTimeouts;
	const float _initialTimeout;

	uint16_t _lastMessageSequenceNumberAcked;
	//Collection of reliable message entries to handle ACKs
	std::vector<ReliableMessageEntry> _reliableMessageEntries;
	unsigned int _reliableMessageEntriesBufferSize;

	std::unordered_map<uint16_t, uint16_t> _unackedMessagesSendTimes;
	std::queue<uint16_t> _messagesRTTToProcess;
	uint16_t _rttMilliseconds;

	//Collection of messages waiting for a previous message in order to guarantee ordered delivery
	std::list<Message*> _orderedMessagesWaitingForPrevious;
	//Next message sequence number expected to guarantee ordered transmission
	unsigned int _nextOrderedMessageSequenceNumber;

	bool _areUnsentACKs;

	bool AreUnackedMessagesToResend() const;
	Message* GetUnackedMessageToResend();
	int GetNextUnackedMessageIndexToResend() const;
	void AddUnackedReliableMessage(Message* message);

	void AckReliableMessage(uint16_t messageSequenceNumber);
	bool DoesUnorderedMessagesContainsSequence(uint16_t sequence, unsigned int& index) const;
	bool AddOrderedMessage(Message* message);
	bool TryRemoveUnackedReliableMessageFromSequence(uint16_t sequence);
	int GetPendingUnackedReliableMessageIndexFromSequence(uint16_t sequence) const;
	Message* DeleteUnackedReliableMessageAtIndex(unsigned int index);

	const ReliableMessageEntry& GetReliableMessageEntry(uint16_t sequenceNumber) const;
	unsigned int GetRollingBufferIndex(uint16_t index) const { return index % _reliableMessageEntriesBufferSize; };

	void AddMessageRTTValueToProcess(uint16_t messageRTT);
	void UpdateRTT();
	float GetRetransmissionTimeout() const;

	void ClearMessages();
};

