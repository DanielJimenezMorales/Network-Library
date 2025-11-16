#pragma once
#include <list>
#include <unordered_map>

#include "transmission_channels/transmission_channel.h"

namespace NetLib
{
	struct MessageHeader;

	struct ReliableMessageEntry
	{
			ReliableMessageEntry()
			    : isAcked( false )
			    , sequenceNumber( 0 )
			{
			}

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
			ReliableOrderedChannel( const ReliableOrderedChannel& ) = delete;
			ReliableOrderedChannel( ReliableOrderedChannel&& other ) noexcept;

			ReliableOrderedChannel& operator=( const ReliableOrderedChannel& ) = delete;
			ReliableOrderedChannel& operator=( ReliableOrderedChannel&& other ) noexcept;

			bool CreateAndSendPacket( Socket& socket, const Address& address,
			                          Metrics::MetricsHandler* metrics_handler ) override;

			bool ArePendingMessagesToSend() const override;
			std::unique_ptr< Message > GetMessageToSend( Metrics::MetricsHandler* metrics_handler );

			bool AddReceivedMessage( std::unique_ptr< Message > message,
			                         Metrics::MetricsHandler* metrics_handler ) override;

			void ProcessACKs( uint32 acks, uint16 lastAckedMessageSequenceNumber,
			                  Metrics::MetricsHandler* metrics_handler ) override;

			void Update( float32 deltaTime, Metrics::MetricsHandler* metrics_handler ) override;

			void Reset() override;

			~ReliableOrderedChannel();

		private:
			/// <summary>
			/// Checks if a message associated with a sequence number is duplicated. This is done by checking if the
			/// message has already been acked.
			/// </summary>
			/// <param name="sequence_number">The sequence number associated with the message.</param>
			/// <returns>True if the message is duplicated, False otherwise.</returns>
			bool IsMessageDuplicated( uint16 sequence_number ) const;

			/// <summary>
			/// Gets the size of the next message pending to be sent. This method returns 0 if there aren't any pending
			/// messages to en sent.
			/// </summary>
			/// <returns>A positive number with the size of the next unsent message or 0 if there are no messages to be
			/// sent.</returns>
			uint32 GetSizeOfNextUnsentMessage() const;

			/// <summary>
			/// Checks if the message can be used by this channel.
			/// </summary>
			/// <param name="header">The header of the message to check.</param>
			/// <returns>True if it is suitable, False otherwise.</returns>
			bool IsMessageSuitable( const MessageHeader& header ) const override;

			//////////
			// ACKS
			//////////

			/// <summary>
			/// Generates the ACK bits to be sent in the next packet.
			/// </summary>
			/// <returns>The ACK bits.</returns>
			uint32 GenerateACKs() const;

			/// <summary>
			/// Acks a reliable message from the remote peer associated to the given sequence number. This will be used
			/// for later send back the ACK of this message saying that we've received it.
			/// </summary>
			/// <param name="sequence_number">The sequence number associated with the remote peer message.</param>
			void AckReliableMessage( uint16 sequence_number );

			/// <summary>
			/// Gets the remote peer reliable message entry associated with the given sequence number.
			/// </summary>
			/// <param name="sequence_number">The sequence number associated to the message we want to get.</param>
			/// <returns>The Reliable message entry associated with sequence_number</returns>
			const ReliableMessageEntry& GetRemotePeerReliableMessageEntry( uint16 sequence_number ) const;

			uint32 GetRollingBufferIndex( uint16 index ) const { return index % _reliableMessageEntriesBufferSize; };

			//////////////////////
			// UNACKED MESSAGES
			//////////////////////

			/// <summary>
			/// Checks if there are any unacked messages that need to be resent.
			/// </summary>
			/// <returns>True if there are unacked messages to be resent, False otherwise.</returns>
			bool AreUnackedMessagesToResend() const;

			/// <summary>
			/// Gets, if available, the first unacked message that is considered lost and needs to be resent. If there
			/// are no unacked messages to be resent this method returns nullptr. See also AreUnackedMessagesToResend().
			/// </summary>
			/// <returns>The message to be resent if available or nullptr otherwise.</returns>
			std::unique_ptr< Message > TryGetUnackedMessageToResend();

			/// <summary>
			/// Gets, if available, the first _unackedReliableMessageTimeouts index that has timed out and needs to be
			/// resent. If there are no unacked messages to be resent this method returns -1.
			/// </summary>
			/// <returns>The _unackedReliableMessageTimeouts index to be resent if available or -1 otherwise.</returns>
			int32 TryGetNextUnackedMessageIndexToResend() const;

			/// <summary>
			/// Adds an unacked message to the unacked messages buffer (_unackedReliableMessages). Also, calculates the
			/// dynamic timeout for that unacked message and adds it to the _unackedReliableMessageTimeouts buffer.
			/// </summary>
			/// <param name="message">The message to be tagged as unacked.</param>
			void AddUnackedMessage( std::unique_ptr< Message > message );

			/// <summary>
			///	Removes, if available, from the _unackedReliableMessages buffer an unacked message that has been acked
			/// by the remote peer.
			/// </summary>
			/// <param name="sequence_number">The sequence number of the acked message.</param>
			/// <param name="metrics_handler">A pointer to the metrics handler to update LATENCY and JITTER
			/// metrics.</param>
			/// <returns>True if the acked message was removed from _unackedReliableMessages, False otherwise.</returns>
			bool TryRemoveAckedMessageFromUnacked( uint16 sequence_number, Metrics::MetricsHandler* metrics_handler );

			/// <summary>
			/// Gets, if available, from the _unackedReliableMessages buffer the unacked message index associated to
			/// sequence_number. If no unacked message associated with that sequence number is found this method returns
			/// -1.
			/// </summary>
			/// <param name="sequence_number">The sequence number associated with the unacked message we want to
			/// get.</param> <returns>The _unackedReliableMessages index to be resent if available or -1
			/// otherwise.</returns>
			int32 TryGetUnackedMessageIndex( uint16 sequence_number ) const;

			/// <summary>
			/// Removes from the _unackedReliableMessages buffer a message at the specified index and returns it.
			/// </summary>
			/// <param name="index">The buffer index where the message to be deleted is at.</param>
			/// <returns>The message that was erased from the buffer.</returns>
			std::unique_ptr< Message > RemoveUnackedMessageFromBufferAtIndex( uint32 index );

			/// <summary>
			/// Sets the unacked message send time for the given sequence number. This is used to calculate the RTT when
			/// the ACK arrives.
			/// </summary>
			/// <param name="sequence_number">The sequence number of the unacked message we want to set the send
			/// time.</param>
			void SetUnackedMessageSendTime( uint16 sequence_number );

			////////////////////////
			// UNORDERED MESSAGES
			////////////////////////

			/// <summary>
			///
			/// </summary>
			/// <param name="message"></param>
			void ProcessOrderedMessage( std::unique_ptr< Message > message );

			/// <summary>
			/// Processes an unordered message received. This means that the message is not the one we were expecting
			/// but a newer one. So we need to wait for the expected message before processing newer ones.
			/// </summary>
			/// <param name="message">The unordered message received</param>
			/// <param name="metrics_handler">The metrics handler to update out of order metric metric</param>
			void ProcessUnorderedMessage( std::unique_ptr< Message > message,
			                              Metrics::MetricsHandler* metrics_handler );

			/// <summary>
			/// Adds an unordered message to the _unorderedMessagesWaitingForPrevious buffer until you get them in the
			/// correct order. The message will wait there until all previous messages are received.
			/// </summary>
			/// <param name="message">The unordered message to buffer.</param>
			void AddUnorderedMessage( std::unique_ptr< Message > message );

			/// <summary>
			/// Checks if the message associated to sequence_number is inside the _unorderedMessagesWaitingForPrevious
			/// buiffer
			/// </summary>
			/// <param name="sequence_number">The sequence number associated with the message you want to look
			/// for</param>
			/// <param name="index">[Out Parameter] The buffer index where the message is located</param>
			/// <returns>True if the message was found (check out_index for position), False otherwise.</returns>
			bool DoesUnorderedMessagesBufferContainsSequenceNumber( uint16 sequence_number, uint32& out_index ) const;

			////////
			// RTT
			////////

			/// <summary>
			/// Updates the channel's RTT value based on the samples calculated in the past. This is useful for
			/// calculating the dynamic retransmission timeout.
			/// </summary>
			void UpdateRTT( uint32 message_rtt );

			/// <summary>
			/// Gets the dynamic retransmission timeout at the time this method is called. This timeout might change
			/// over time based on the RTT.
			/// </summary>
			/// <returns>The retransmission timeout</returns>
			float32 GetRetransmissionTimeout() const;

			/// <summary>
			/// Deallocates all message-related buffers.
			/// </summary>
			void ClearMessages();

			// RELIABLE RELATED

			const uint32 ACK_BITS_SIZE = 32;

			/// <summary>
			/// Retransmission timeout when RTT is zero (At the beginning of the game for example)
			/// </summary>
			const float32 INITIAL_TIMEOUT = 0.5f;

			/// <summary>
			/// Reliable messages that have not already been acked
			/// </summary>
			std::list< std::unique_ptr< Message > > _unackedReliableMessages;

			/// <summary>
			/// Timeouts of _unackedReliableMessages. This is used to determine when a message needs to be resent. When
			/// the message's timeout expires, it will be considered lost and as a consequence a resend will happen.
			/// </summary>
			std::list< float32 > _unackedReliableMessageTimeouts;

			/// <summary>
			/// Flag to check if there are pending ACKs to send. This will allow us to not wait until there's a message
			/// to be sent if we have unsent ACK bits.
			/// </summary>
			bool _areUnsentACKs;

			/// <summary>
			/// Last reliable message sequence acked
			/// </summary>
			uint16 _lastAckedMessageSequenceNumber;

			/// <summary>
			/// Reliable entries to handle remote peer message ACKs
			/// </summary>
			std::vector< ReliableMessageEntry > _remotePeerReliableMessageEntries;
			uint32 _reliableMessageEntriesBufferSize;

			/// <summary>
			/// Elapsed time since start of the program that each reliable message was sent (For RTT calculation
			/// purposes)
			/// </summary>
			std::unordered_map< uint16, uint32 > _unackedMessagesSendTimes;

			// RTT RELATED

			/// <summary>
			/// Current RTT value in milliseconds
			/// </summary>
			uint32 _rttMilliseconds;

			// ORDERED RELATED

			/// <summary>
			/// Messages waiting for a previous message in order to guarantee ordered delivery
			/// </summary>
			std::list< std::unique_ptr< Message > > _unorderedMessagesWaitingForPrevious;

			/// <summary>
			/// Next message sequence number expected to guarantee ordered transmission
			/// </summary>
			uint32 _nextOrderedMessageSequenceNumber;
	};
} // namespace NetLib
