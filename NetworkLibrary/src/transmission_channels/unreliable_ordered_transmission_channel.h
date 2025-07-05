#pragma once
#include "transmission_channels/transmission_channel.h"

namespace NetLib
{
	struct MessageHeader;

	class UnreliableOrderedTransmissionChannel : public TransmissionChannel
	{
		public:
			UnreliableOrderedTransmissionChannel();
			UnreliableOrderedTransmissionChannel( const UnreliableOrderedTransmissionChannel& ) = delete;
			UnreliableOrderedTransmissionChannel( UnreliableOrderedTransmissionChannel&& other ) noexcept;

			UnreliableOrderedTransmissionChannel& operator=( const UnreliableOrderedTransmissionChannel& ) = delete;
			UnreliableOrderedTransmissionChannel& operator=( UnreliableOrderedTransmissionChannel&& other ) noexcept;

			bool CreateAndSendPacket( Socket& socket, const Address& address,
			                          Metrics::MetricsHandler* metrics_handler ) override;

			bool AddMessageToSend( std::unique_ptr< Message > message ) override;
			bool ArePendingMessagesToSend() const override;
			std::unique_ptr< Message > GetMessageToSend( Metrics::MetricsHandler* metrics_handler );
			uint32 GetSizeOfNextUnsentMessage() const;

			bool AddReceivedMessage( std::unique_ptr< Message > message,
			                         Metrics::MetricsHandler* metrics_handler ) override;
			bool ArePendingReadyToProcessMessages() const override;
			const Message* GetReadyToProcessMessage() override;

			void ProcessACKs( uint32 acks, uint16 lastAckedMessageSequenceNumber,
			                  Metrics::MetricsHandler* metrics_handler ) override;
			bool IsMessageDuplicated( uint16 messageSequenceNumber ) const;

			void Update( float32 deltaTime, Metrics::MetricsHandler* metrics_handler ) override;

			void Reset() override;

		private:
			uint16 _lastMessageSequenceNumberReceived;

			bool IsSequenceNumberNewerThanLastReceived( uint16 sequenceNumber ) const;

			/// <summary>
			/// Checks if the message can be used by this channel.
			/// </summary>
			/// <param name="header">The header of the message to check.</param>
			/// <returns>True if it is suitable, False otherwise.</returns>
			bool IsMessageSuitable( const MessageHeader& header ) const;
	};
} // namespace NetLib
