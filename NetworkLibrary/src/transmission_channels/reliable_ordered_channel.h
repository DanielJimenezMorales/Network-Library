#pragma once
#include <list>
#include <unordered_map>

#include "transmission_channels/transmission_channel.h"

namespace NetLib
{
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
		uint16 sequenceNumber;
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

		void Reset() override;

		uint32 GetRTTMilliseconds() const override;

		~ReliableOrderedChannel();

	protected:
		void FreeSentMessage(MessageFactory& messageFactory, std::unique_ptr<Message> message) override;

	private:
		//RELIABLE RELATED
		//Collection of reliable messages that have not already been acked
		std::list<std::unique_ptr<Message>> _unackedReliableMessages;
		//Timeouts of _unackedReliableMessages
		std::list<float32> _unackedReliableMessageTimeouts;
		//Retransmission timeout when RTT is zero
		const float32 _initialTimeout = 0.5f;
		//Flag to check if are there pending ACKs to send
		bool _areUnsentACKs;
		//Last reliable message sequence acked
		uint16 _lastMessageSequenceNumberAcked;
		//Collection of reliable message entries to handle ACKs
		std::vector<ReliableMessageEntry> _reliableMessageEntries;
		uint32 _reliableMessageEntriesBufferSize;
		//Elapsed time since start of the program that each reliable message was sent (For RTT purposes)
		std::unordered_map<uint16, uint16> _unackedMessagesSendTimes;

		//RTT RELATED
		//Message RTT values waiting to be added to the current RTT value
		std::queue<uint16> _messagesRTTToProcess;
		//Current RTT value in milliseconds
		uint16 _rttMilliseconds;

		//ORDERED RELATED
		//Collection of messages waiting for a previous message in order to guarantee ordered delivery
		std::list<std::unique_ptr<Message>> _orderedMessagesWaitingForPrevious;
		//Next message sequence number expected to guarantee ordered transmission
		uint32 _nextOrderedMessageSequenceNumber;

		bool AreUnackedMessagesToResend() const;
		std::unique_ptr<Message> GetUnackedMessageToResend();
		int32 GetNextUnackedMessageIndexToResend() const;
		void AddUnackedReliableMessage(std::unique_ptr<Message> message);

		void AckReliableMessage(uint16 messageSequenceNumber);
		bool DoesUnorderedMessagesContainsSequence(uint16 sequence, uint32& index) const;
		bool AddOrderedMessage(std::unique_ptr<Message> message);
		bool TryRemoveUnackedReliableMessageFromSequence(uint16 sequence);
		int32 GetPendingUnackedReliableMessageIndexFromSequence(uint16 sequence) const;
		std::unique_ptr<Message> DeleteUnackedReliableMessageAtIndex(uint32 index);

		const ReliableMessageEntry& GetReliableMessageEntry(uint16 sequenceNumber) const;
		uint32 GetRollingBufferIndex(uint16 index) const { return index % _reliableMessageEntriesBufferSize; };

		void AddMessageRTTValueToProcess(uint16 messageRTT);
		void UpdateRTT();
		float32 GetRetransmissionTimeout() const;

		void ClearMessages();
	};
}
