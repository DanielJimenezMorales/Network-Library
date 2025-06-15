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

			void AddMessageToSend( std::unique_ptr< Message > message ) override;
			bool ArePendingMessagesToSend() const override;
			std::unique_ptr< Message > GetMessageToSend( Metrics::MetricsHandler* metrics_handler ) override;
			uint32 GetSizeOfNextUnsentMessage() const override;

			void AddReceivedMessage( std::unique_ptr< Message > message,
			                         Metrics::MetricsHandler* metrics_handler ) override;
			bool ArePendingReadyToProcessMessages() const override;
			const Message* GetReadyToProcessMessage() override;

			void SeUnsentACKsToFalse() override;
			bool AreUnsentACKs() const override;
			uint32 GenerateACKs() const override;
			void ProcessACKs( uint32 acks, uint16 lastAckedMessageSequenceNumber,
			                  Metrics::MetricsHandler* metrics_handler ) override;
			bool IsMessageDuplicated( uint16 messageSequenceNumber ) const override;

			void Update( float32 deltaTime ) override;

			uint16 GetLastMessageSequenceNumberAcked() const override;

			uint32 GetRTTMilliseconds() const override;

			void Reset() override;

			~UnreliableOrderedTransmissionChannel();

		protected:
			void FreeSentMessage( MessageFactory& messageFactory, std::unique_ptr< Message > message ) override;

		private:
			uint16 _lastMessageSequenceNumberReceived;

			bool IsSequenceNumberNewerThanLastReceived( uint16 sequenceNumber ) const;
	};
} // namespace NetLib
