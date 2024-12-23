#pragma once
#include "NumericTypes.h"

#include <queue>
#include <vector>
#include <memory>

namespace NetLib
{
	class Message;
	class MessageFactory;

	enum TransmissionChannelType : uint8
	{
		UnreliableOrdered = 0,
		ReliableOrdered = 1,
		UnreliableUnordered = 2
	};

	class TransmissionChannel
	{
		public:
			TransmissionChannel( TransmissionChannelType type );
			TransmissionChannel( const TransmissionChannel& ) = delete;
			TransmissionChannel( TransmissionChannel&& other ) noexcept;

			TransmissionChannel& operator=( const TransmissionChannel& ) = delete;
			TransmissionChannel& operator=( TransmissionChannel&& other ) noexcept;

			TransmissionChannelType GetType() { return _type; }

			virtual void AddMessageToSend( std::unique_ptr< Message > message ) = 0;
			virtual bool ArePendingMessagesToSend() const = 0;
			virtual std::unique_ptr< Message > GetMessageToSend() = 0;
			virtual uint32 GetSizeOfNextUnsentMessage() const = 0;
			void AddSentMessage( std::unique_ptr< Message > message );
			void FreeSentMessages();

			virtual void AddReceivedMessage( std::unique_ptr< Message > message ) = 0;
			virtual bool ArePendingReadyToProcessMessages() const = 0;
			virtual const Message* GetReadyToProcessMessage() = 0;
			void FreeProcessedMessages();

			virtual void SeUnsentACKsToFalse() = 0; // This method should not exists and be done automatically. However,
			                                        // I have not found how so for now, we do it manually.
			virtual bool AreUnsentACKs() const = 0;
			virtual uint32 GenerateACKs() const = 0;
			virtual void ProcessACKs( uint32 acks, uint16 lastAckedMessageSequenceNumber ) = 0;
			virtual bool IsMessageDuplicated( uint16 messageSequenceNumber ) const = 0;

			virtual void Update( float32 deltaTime ) = 0;

			virtual uint16 GetLastMessageSequenceNumberAcked() const = 0;

			virtual void Reset();

			virtual uint32 GetRTTMilliseconds() const = 0;

			virtual ~TransmissionChannel();

		protected:
			// Collection of messages that are waiting to be sent.
			std::vector< std::unique_ptr< Message > > _unsentMessages;
			// Collection of messages that have been sent and are waiting to be released (Used for memory management
			// purposes)
			std::queue< std::unique_ptr< Message > > _sentMessages;
			// Collection of received messages ready to be processed
			std::queue< std::unique_ptr< Message > > _readyToProcessMessages;
			// Collection of messages that have been processed and are waiting to be released (Used for memory
			// management purposes)
			std::queue< std::unique_ptr< Message > > _processedMessages;

			virtual void FreeSentMessage( MessageFactory& messageFactory, std::unique_ptr< Message > message ) = 0;

			uint16 GetNextMessageSequenceNumber() const { return _nextMessageSequenceNumber; }
			void IncreaseMessageSequenceNumber() { ++_nextMessageSequenceNumber; };

		private:
			TransmissionChannelType _type;
			uint16 _nextMessageSequenceNumber;

			void ClearMessages();
	};
} // namespace NetLib
