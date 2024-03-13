#pragma once
#include <list>
#include <unordered_map>

#include "TransmissionChannel.h"

class Message;

struct ReliableMessageEntry
{
	ReliableMessageEntry() : isAcked(false), sequenceNumber(0) {}

	void Reset()
	{
		isAcked = false;
		sequenceNumber = 0;
	}

	bool isAcked;
	uint16_t sequenceNumber;
};

class ReliableOrderedChannel : public TransmissionChannel
{
public:
	ReliableOrderedChannel();
	ReliableOrderedChannel(const ReliableOrderedChannel&) = delete;
	ReliableOrderedChannel(ReliableOrderedChannel&& other) noexcept;

	ReliableOrderedChannel& operator=(const ReliableOrderedChannel&) = delete;
	ReliableOrderedChannel& operator=(ReliableOrderedChannel&& other) noexcept;

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

	void Reset() override;

	unsigned int GetRTTMilliseconds() const override;

	~ReliableOrderedChannel();

protected:
	void FreeSentMessage(MessageFactory& messageFactory, std::unique_ptr<Message> message) override;

private:
	//RELIABLE RELATED
	//Collection of reliable messages that have not already been acked
	std::list<std::unique_ptr<Message>> _unackedReliableMessages;
	//Timeouts of _unackedReliableMessages
	std::list<float> _unackedReliableMessageTimeouts;
	//Retransmission timeout when RTT is zero
	const float _initialTimeout = 0.5f;
	//Flag to check if are there pending ACKs to send
	bool _areUnsentACKs;
	//Last reliable message sequence acked
	uint16_t _lastMessageSequenceNumberAcked;
	//Collection of reliable message entries to handle ACKs
	std::vector<ReliableMessageEntry> _reliableMessageEntries;
	unsigned int _reliableMessageEntriesBufferSize;
	//Elapsed time since start of the program that each reliable message was sent (For RTT purposes)
	std::unordered_map<uint16_t, uint16_t> _unackedMessagesSendTimes;

	//RTT RELATED
	//Message RTT values waiting to be added to the current RTT value
	std::queue<uint16_t> _messagesRTTToProcess;
	//Current RTT value in milliseconds
	uint16_t _rttMilliseconds;

	//ORDERED RELATED
	//Collection of messages waiting for a previous message in order to guarantee ordered delivery
	std::list<std::unique_ptr<Message>> _orderedMessagesWaitingForPrevious;
	//Next message sequence number expected to guarantee ordered transmission
	unsigned int _nextOrderedMessageSequenceNumber;

	bool AreUnackedMessagesToResend() const;
	std::unique_ptr<Message> GetUnackedMessageToResend();
	int GetNextUnackedMessageIndexToResend() const;
	void AddUnackedReliableMessage(std::unique_ptr<Message> message);

	void AckReliableMessage(uint16_t messageSequenceNumber);
	bool DoesUnorderedMessagesContainsSequence(uint16_t sequence, unsigned int& index) const;
	bool AddOrderedMessage(std::unique_ptr<Message> message);
	bool TryRemoveUnackedReliableMessageFromSequence(uint16_t sequence);
	int GetPendingUnackedReliableMessageIndexFromSequence(uint16_t sequence) const;
	std::unique_ptr<Message> DeleteUnackedReliableMessageAtIndex(unsigned int index);

	const ReliableMessageEntry& GetReliableMessageEntry(uint16_t sequenceNumber) const;
	unsigned int GetRollingBufferIndex(uint16_t index) const { return index % _reliableMessageEntriesBufferSize; };

	void AddMessageRTTValueToProcess(uint16_t messageRTT);
	void UpdateRTT();
	float GetRetransmissionTimeout() const;

	void ClearMessages();
};

