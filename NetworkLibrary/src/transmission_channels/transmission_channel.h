#pragma once
#include "numeric_types.h"

#include <queue>
#include <vector>
#include <memory>

namespace NetLib
{
	class Message;
	class MessageFactory;
	class Socket;
	class Address;

	namespace Metrics
	{
		class MetricsHandler;
	}

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

			/// <summary>
			/// Creates a packet with pending data and sends it through the socket to the specified address.
			/// </summary>
			virtual bool GenerateAndSerializePacket( Socket& socket, const Address& address,
			                                         Metrics::MetricsHandler* metrics_handler ) = 0;

			virtual void AddMessageToSend( std::unique_ptr< Message > message ) = 0;
			virtual uint32 GetSizeOfNextUnsentMessage() const = 0;

			virtual void AddReceivedMessage( std::unique_ptr< Message > message,
			                                 Metrics::MetricsHandler* metrics_handler ) = 0;
			virtual bool ArePendingReadyToProcessMessages() const = 0;
			virtual const Message* GetReadyToProcessMessage() = 0;
			void FreeProcessedMessages();

			virtual void ProcessACKs( uint32 acks, uint16 lastAckedMessageSequenceNumber,
			                          Metrics::MetricsHandler* metrics_handler ) = 0;
			virtual bool IsMessageDuplicated( uint16 messageSequenceNumber ) const = 0;

			virtual void Update( float32 deltaTime ) = 0;

			virtual void Reset();

			virtual ~TransmissionChannel();

		protected:
			// Collection of messages that are waiting to be sent.
			std::vector< std::unique_ptr< Message > > _unsentMessages;
			// Collection of received messages ready to be processed
			std::queue< std::unique_ptr< Message > > _readyToProcessMessages;
			// Collection of messages that have been processed and are waiting to be released (Used for memory
			// management purposes)
			std::queue< std::unique_ptr< Message > > _processedMessages;

			uint16 GetNextMessageSequenceNumber() const { return _nextMessageSequenceNumber; }
			void IncreaseMessageSequenceNumber() { ++_nextMessageSequenceNumber; };

		private:
			TransmissionChannelType _type;
			uint16 _nextMessageSequenceNumber;

			void ClearMessages();
	};
} // namespace NetLib
