#pragma once
#include <memory>

#include "communication/message.h"

#include "transmission_channels/transmission_channel.h"

namespace NetLib
{
	struct MessageHeader;

	class UnreliableUnorderedTransmissionChannel : public TransmissionChannel
	{
		public:
			UnreliableUnorderedTransmissionChannel( MessageFactory* message_factory );
			UnreliableUnorderedTransmissionChannel( const UnreliableUnorderedTransmissionChannel& ) = delete;
			UnreliableUnorderedTransmissionChannel( UnreliableUnorderedTransmissionChannel&& other ) noexcept;

			UnreliableUnorderedTransmissionChannel& operator=( const UnreliableUnorderedTransmissionChannel& ) = delete;
			UnreliableUnorderedTransmissionChannel& operator=(
			    UnreliableUnorderedTransmissionChannel&& other ) noexcept;

			bool CreateAndSendPacket( Socket& socket, const Address& address,
			                          Metrics::MetricsHandler* metrics_handler ) override;

			bool ArePendingMessagesToSend() const override;
			std::unique_ptr< Message > GetMessageToSend( Metrics::MetricsHandler* metrics_handler );
			uint32 GetSizeOfNextUnsentMessage() const;

			bool AddReceivedMessage( std::unique_ptr< Message > message,
			                         Metrics::MetricsHandler* metrics_handler ) override;

			void ProcessACKs( uint32 acks, uint16 lastAckedMessageSequenceNumber,
			                  Metrics::MetricsHandler* metrics_handler ) override;
			bool IsMessageDuplicated( uint16 messageSequenceNumber ) const;

			void Update( float32 deltaTime, Metrics::MetricsHandler* metrics_handler ) override;

		private:
			/// <summary>
			/// Checks if the message can be used by this channel.
			/// </summary>
			/// <param name="header">The header of the message to check.</param>
			/// <returns>True if it is suitable, False otherwise.</returns>
			bool IsMessageSuitable( const MessageHeader& header ) const override;
	};
} // namespace NetLib
