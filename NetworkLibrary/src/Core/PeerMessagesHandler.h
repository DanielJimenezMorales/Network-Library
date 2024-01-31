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

	bool AreUnackedReliableMessages() const { return !_unackedReliableMessages.empty(); };
	Message* GetPendingACKReliableMessage();

	bool AddReceivedMessage(Message* message);
	bool ArePendingReadyToProcessMessages() const { return !_readyToProcessMessages.empty(); };
	const Message* GetReadyToProcessMessage();
	bool DoesUnorderedMessagesContainsSequence(uint16_t sequence, unsigned int& index) const;

	bool AddOrderedMessage(Message* message);

	unsigned int GetSizeOfNextPendingMessage() const;

	/// <summary>
	/// Call this after sending a packet to this peer. This will release all the memory related to messages sent (Only if they are non reliable)
	/// </summary>
	void FreeSentMessages();
	/// <summary>
	/// Call this after processing all the messages from this peer. This will release all the memory related to messages processed.
	/// </summary>
	void FreeProcessedMessages();
	void ClearMessages();

	uint32_t GenerateACKs() const;
	void ProcessACKs(uint32_t acks, uint16_t lastAckedMessageSequenceNumber);
	uint16_t GetLastMessageSequenceNumberAcked() const { return _lastMessageSequenceNumberAcked; };
	void AckReliableMessage(uint16_t messageSequenceNumber);
	bool IsMessageDuplicated(uint16_t messageSequenceNumber) const;

	~PeerMessagesHandler();

private:
	//Collection of messages that are waiting to be sent.
	std::vector<Message*> _pendingMessages;
	//Collection of messages that have been sent and are waiting to be released (Used for memory management purposes)
	std::queue<Message*> _sentMessages;
	//Collection of received messages ready to be processed
	std::queue<Message*> _readyToProcessMessages;
	//Collection of messages that have been processed and are waiting to be released (Used for memory management purposes)
	std::queue<Message*> _processedMessages;

	//*************************
	//RELIABLE MESSAGES RELATED
	//*************************
	//Collection of reliable messages that have not already been acked
	std::list<Message*> _unackedReliableMessages;
	//Last reliable message sequence acked
	uint16_t _lastMessageSequenceNumberAcked;
	//Collection of reliable message entries to handle ACKs
	std::vector<ReliableMessageEntry> _reliableMessageEntries;
	unsigned int _reliableMessageEntriesBufferSize;

	//*************************
	//ORDERED MESSAGES RELATED
	//*************************
	//Collection of messages waiting for a previous message in order to guarantee ordered delivery
	std::list<Message*> _orderedMessagesWaitingForPrevious;
	//Next message sequence number expected to guarantee ordered transmission
	unsigned int _nextOrderedMessageSequenceNumber;

	const ReliableMessageEntry& GetReliableMessageEntry(uint16_t sequenceNumber) const;
	bool TryRemoveUnackedReliableMessageFromSequence(uint16_t sequence);
	int GetPendingUnackedReliableMessageIndexFromSequence(uint16_t sequence) const;
	void DeleteUnackedReliableMessageAtIndex(unsigned int index);
	unsigned int GetRollingBufferIndex(uint16_t index) const {return index % _reliableMessageEntriesBufferSize;};
};

