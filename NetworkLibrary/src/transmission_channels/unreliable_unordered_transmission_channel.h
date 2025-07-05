#pragma once
#include <memory>

#include "communication/message.h"

#include "transmission_channels/transmission_channel.h"

namespace NetLib
{
	class MessageFactory;

	class UnreliableUnorderedTransmissionChannel : public TransmissionChannel
	{
		public:
			UnreliableUnorderedTransmissionChannel();
			UnreliableUnorderedTransmissionChannel( const UnreliableUnorderedTransmissionChannel& ) = delete;
			UnreliableUnorderedTransmissionChannel( UnreliableUnorderedTransmissionChannel&& other ) noexcept;

			UnreliableUnorderedTransmissionChannel& operator=( const UnreliableUnorderedTransmissionChannel& ) = delete;
			UnreliableUnorderedTransmissionChannel& operator=(
			    UnreliableUnorderedTransmissionChannel&& other ) noexcept;

			bool GenerateAndSerializePacket( Socket& socket, const Address& address,
			                                 Metrics::MetricsHandler* metrics_handler ) override;

			void AddMessageToSend( std::unique_ptr< Message > message ) override;
			bool ArePendingMessagesToSend() const override;
			std::unique_ptr< Message > GetMessageToSend( Metrics::MetricsHandler* metrics_handler );
			uint32 GetSizeOfNextUnsentMessage() const override;

			void AddReceivedMessage( std::unique_ptr< Message > message,
			                         Metrics::MetricsHandler* metrics_handler ) override;
			bool ArePendingReadyToProcessMessages() const override;
			const Message* GetReadyToProcessMessage() override;

			void ProcessACKs( uint32 acks, uint16 lastAckedMessageSequenceNumber,
			                  Metrics::MetricsHandler* metrics_handler ) override;
			bool IsMessageDuplicated( uint16 messageSequenceNumber ) const override;

			void Update( float32 deltaTime, Metrics::MetricsHandler* metrics_handler ) override;
	};
} // namespace NetLib
