#pragma once
#include <vector>
#include <queue>
#include <list>
#include <cstdint>

class Message;

struct ReliableMessageEntry
{
	ReliableMessageEntry() : isAcked(false), sequenceNumber(0) {}

	bool isAcked;
	uint16_t sequenceNumber;
};

class PeerMessagesHandler
{
public:
	PeerMessagesHandler();

	/// <summary>
	/// Add a message to the pending messages to send collection.
	/// </summary>
	/// <param name="message">The message to send</param>
	/// <returns>True if the operation succeed</returns>
	bool AddMessage(Message* message);
	bool ArePendingMessages() const { return !_pendingMessages.empty(); }

	/// <summary>
	/// Get a message from the pending messages to send collection. IMPORTANT: DO NOT FREE THIS MEMORY. Instead, call FreeSentMessages() once the packet
	/// has been sent.
	/// </summary>
	/// <returns>A pointer to a pending message to send</returns>
	Message* GetPendingMessage();

	bool ArePendingACKReliableMessages() const { return !_pendingAckReliableMessages.empty(); };
	Message* GetPendingACKReliableMessage();

	bool AddReceivedMessage(Message* message);
	bool ArePendingReadyToProcessMessages() const { return !_pendingMessagesReadyToProcess.empty(); };
	Message* GetPendingReadyToProcessMessage();
	bool DoesUnorderedMessagesContainsSequence(uint16_t sequence, unsigned int& index) const;

	bool AddOrderedMessage(Message* message);

	unsigned int GetSizeOfNextPendingMessage() const;

	/// <summary>
	/// Call this after sending a packet to this client. This will release all the memory related to messages sent (Only if they are non reliable)
	/// </summary>
	void FreeSentMessages();
	void ClearMessages();

	uint32_t GenerateACKs() const;
	void ProcessACKs(uint32_t acks, uint16_t lastAckedMessageSequenceNumber);
	uint16_t GetLastMessageSequenceNumberAcked() const { return _lastMessageSequenceNumberAcked; };
	void AckReliableMessage(uint16_t messageSequenceNumber);
	bool IsMessageDuplicated(uint16_t messageSequenceNumber) const;

	~PeerMessagesHandler();

private:
	std::vector<Message*> _pendingMessages;
	std::list<Message*> _pendingAckReliableMessages;
	std::queue<Message*> _sentMessages;

	uint16_t _lastMessageSequenceNumberAcked;

	unsigned int _reliableMessageEntriesBufferSize;
	std::vector<ReliableMessageEntry> _reliableMessageEntries;

	unsigned int _nextOrderedMessageSequenceNumber;
	std::list<Message*> _reliableUnorderedMessages;

	std::queue<Message*> _pendingMessagesReadyToProcess;

	const ReliableMessageEntry& GetReliableMessageEntry(uint16_t sequenceNumber) const;
	bool TryRemovePendingACKReliableMessageFromSequence(uint16_t sequence);
	int GetPendingACKReliableMessageIndexFromSequence(uint16_t sequence) const;
	void DeletePendingACKReliableMessageAtIndex(unsigned int index);
	unsigned int GetRollingBufferIndex(uint16_t index) const {return index % _reliableMessageEntriesBufferSize;};
};

