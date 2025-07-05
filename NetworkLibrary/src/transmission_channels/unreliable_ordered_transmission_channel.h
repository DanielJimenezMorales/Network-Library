#pragma once
#include "transmission_channels/transmission_channel.h"

namespace NetLib
{
	class UnreliableOrderedTransmissionChannel : public TransmissionChannel
	{
		public:
			UnreliableOrderedTransmissionChannel();
			UnreliableOrderedTransmissionChannel( const UnreliableOrderedTransmissionChannel& ) = delete;
			UnreliableOrderedTransmissionChannel( UnreliableOrderedTransmissionChannel&& other ) noexcept;

			UnreliableOrderedTransmissionChannel& operator=( const UnreliableOrderedTransmissionChannel& ) = delete;
			UnreliableOrderedTransmissionChannel& operator=( UnreliableOrderedTransmissionChannel&& other ) noexcept;

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

			void Reset() override;

		private:
			uint16 _lastMessageSequenceNumberReceived;

			bool IsSequenceNumberNewerThanLastReceived( uint16 sequenceNumber ) const;
	};
} // namespace NetLib
