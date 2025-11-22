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
			TransmissionChannel( TransmissionChannelType type, MessageFactory* message_factory );
			TransmissionChannel( const TransmissionChannel& ) = delete;
			TransmissionChannel( TransmissionChannel&& other ) noexcept;

			TransmissionChannel& operator=( const TransmissionChannel& ) = delete;
			TransmissionChannel& operator=( TransmissionChannel&& other ) noexcept;

			TransmissionChannelType GetType() { return _type; }

			/// <summary>
			/// Creates a packet with pending data and sends it through the socket to the specified address.
			/// </summary>
			/// <param name="socket">The socket to send the packet through.</param>
			/// <param name="address">The targed address where the packet is going to be sent to.</param>
			/// <param name="metrics_handler">A pointer to the metrics handler to submit any metrics such as
			/// bandwidth.</param>
			/// <returns>True if the packet was created and sent, False otherwise.</returns>
			virtual bool CreateAndSendPacket( Socket& socket, const Address& address,
			                                  Metrics::MetricsHandler* metrics_handler ) = 0;

			/// <summary>
			/// Adds to the channel a message pending to be sent through the network. The header of the message must be
			/// suitable with the channel.
			/// </summary>
			/// <param name="message">The message pending to be sent.</param>
			/// <returns>True if the message was stored correclt, False otherwise.</returns>
			virtual bool AddMessageToSend( std::unique_ptr< Message > message ) = 0;

			/// <summary>
			/// Checks if there are any messages pending to be sent through the network.
			/// </summary>
			/// <returns>True if there are pending messages, False otherwise.</returns>
			virtual bool ArePendingMessagesToSend() const = 0;

			/// <summary>
			/// Adds to the channel a message received to be processed. The header of the message must be
			/// suitable with the channel.
			/// </summary>
			/// <param name="message">The message received to be processed.</param>
			/// <returns>True if the message was stored correclt, False otherwise.</returns>
			virtual bool AddReceivedMessage( std::unique_ptr< Message > message,
			                                 Metrics::MetricsHandler* metrics_handler ) = 0;
			virtual bool ArePendingReadyToProcessMessages() const = 0;
			virtual const Message* GetReadyToProcessMessage() = 0;
			void FreeProcessedMessages();

			virtual void ProcessACKs( uint32 acks, uint16 lastAckedMessageSequenceNumber,
			                          Metrics::MetricsHandler* metrics_handler ) = 0;

			virtual void Update( float32 deltaTime, Metrics::MetricsHandler* metrics_handler ) = 0;

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

			MessageFactory* _messageFactory;

			uint16 GetNextMessageSequenceNumber() const { return _nextMessageSequenceNumber; }
			void IncreaseMessageSequenceNumber() { ++_nextMessageSequenceNumber; };

		private:
			TransmissionChannelType _type;
			uint16 _nextMessageSequenceNumber;

			void ClearMessages();
	};
} // namespace NetLib
