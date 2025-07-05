#include "unreliable_ordered_transmission_channel.h"
#include <memory>

#include "communication/message_factory.h"
#include "communication/network_packet.h"

#include "core/buffer.h"
#include "core/socket.h"
#include "core/address.h"

#include "metrics/metric_names.h"
#include "metrics/metrics_handler.h"
namespace NetLib
{
	UnreliableOrderedTransmissionChannel::UnreliableOrderedTransmissionChannel()
	    : TransmissionChannel( TransmissionChannelType::UnreliableOrdered )
	    , _lastMessageSequenceNumberReceived( 0 )
	{
	}

	UnreliableOrderedTransmissionChannel::UnreliableOrderedTransmissionChannel(
	    UnreliableOrderedTransmissionChannel&& other ) noexcept
	    : TransmissionChannel( std::move( other ) )
	    , _lastMessageSequenceNumberReceived( std::move(
	          other._lastMessageSequenceNumberReceived ) ) // unnecessary move, just in case I change that type
	{
	}

	UnreliableOrderedTransmissionChannel& UnreliableOrderedTransmissionChannel::operator=(
	    UnreliableOrderedTransmissionChannel&& other ) noexcept
	{
		_lastMessageSequenceNumberReceived =
		    std::move( other._lastMessageSequenceNumberReceived ); // unnecessary move, just in case I change that type

		TransmissionChannel::operator=( std::move( other ) );
		return *this;
	}

	bool UnreliableOrderedTransmissionChannel::GenerateAndSerializePacket( Socket& socket, const Address& address,
	                                                                       Metrics::MetricsHandler* metrics_handler )
	{
		bool result = false;

		if ( !ArePendingMessagesToSend() )
		{
			return result;
		}

		NetworkPacket packet;

		// TODO Check somewhere if there is a message larger than the maximum packet size. Log a warning saying that the
		// message will never get sent and delete it.
		// TODO Include data prefix in packet's header and check if the data prefix is correct when receiving a packet

		// Check if we should include a message to the packet
		bool arePendingMessages = true;
		bool isThereCapacityLeft = packet.CanMessageFit( GetSizeOfNextUnsentMessage() );

		while ( arePendingMessages && isThereCapacityLeft )
		{
			// Add message to packet
			std::unique_ptr< Message > message = GetMessageToSend( metrics_handler );
			packet.AddMessage( std::move( message ) );

			// Check if we should include another message to the packet
			arePendingMessages = ArePendingMessagesToSend();
			isThereCapacityLeft = packet.CanMessageFit( GetSizeOfNextUnsentMessage() );
		}

		// Set packet header fields
		packet.SetHeaderACKs( 0 );
		packet.SetHeaderLastAcked( 0 );
		packet.SetHeaderChannelType( GetType() );

		// Serialize packet
		uint8* bufferData = new uint8[ packet.Size() ];
		Buffer buffer( bufferData, packet.Size() );
		packet.Write( buffer );

		// Sends packet
		socket.SendTo( buffer.GetData(), buffer.GetSize(), address );

		// TODO See what happens when the socket couldn't send the packet
		if ( metrics_handler != nullptr )
		{
			metrics_handler->AddValue( Metrics::UPLOAD_BANDWIDTH_METRIC, packet.Size() );
		}

		// Clean messages
		MessageFactory& messageFactory = MessageFactory::GetInstance();
		while ( packet.GetNumberOfMessages() > 0 )
		{
			std::unique_ptr< Message > message = packet.GetMessages();
			messageFactory.ReleaseMessage( std::move( message ) );
		}

		delete[] bufferData;

		result = true;
		return result;
	}

	void UnreliableOrderedTransmissionChannel::AddMessageToSend( std::unique_ptr< Message > message )
	{
		_unsentMessages.push_back( std::move( message ) );
	}

	bool UnreliableOrderedTransmissionChannel::ArePendingMessagesToSend() const
	{
		return ( !_unsentMessages.empty() );
	}

	std::unique_ptr< Message > UnreliableOrderedTransmissionChannel::GetMessageToSend(
	    Metrics::MetricsHandler* metrics_handler )
	{
		if ( !ArePendingMessagesToSend() )
		{
			return nullptr;
		}

		std::unique_ptr< Message > message( std::move( _unsentMessages[ 0 ] ) );
		_unsentMessages.erase( _unsentMessages.begin() );

		const uint16 sequenceNumber = GetNextMessageSequenceNumber();
		IncreaseMessageSequenceNumber();

		message->SetHeaderPacketSequenceNumber( sequenceNumber );

		return std::move( message );
	}

	uint32 UnreliableOrderedTransmissionChannel::GetSizeOfNextUnsentMessage() const
	{
		if ( !ArePendingMessagesToSend() )
		{
			return 0;
		}

		return _unsentMessages.front()->Size();
	}

	void UnreliableOrderedTransmissionChannel::AddReceivedMessage( std::unique_ptr< Message > message,
	                                                               Metrics::MetricsHandler* metrics_handler )
	{
		if ( !IsSequenceNumberNewerThanLastReceived( message->GetHeader().messageSequenceNumber ) )
		{
			MessageFactory& messageFactory = MessageFactory::GetInstance();
			messageFactory.ReleaseMessage( std::move( message ) );
			return;
		}

		_lastMessageSequenceNumberReceived = message->GetHeader().messageSequenceNumber;
		_readyToProcessMessages.push( std::move( message ) );
	}

	bool UnreliableOrderedTransmissionChannel::ArePendingReadyToProcessMessages() const
	{
		return ( !_readyToProcessMessages.empty() );
	}

	const Message* UnreliableOrderedTransmissionChannel::GetReadyToProcessMessage()
	{
		if ( !ArePendingReadyToProcessMessages() )
		{
			return nullptr;
		}

		std::unique_ptr< Message > message( std::move( _readyToProcessMessages.front() ) );
		_readyToProcessMessages.pop();

		Message* messageToReturn = message.get();
		_processedMessages.push( std::move( message ) );

		return messageToReturn;
	}

	void UnreliableOrderedTransmissionChannel::ProcessACKs( uint32 acks, uint16 lastAckedMessageSequenceNumber,
	                                                        Metrics::MetricsHandler* metrics_handler )
	{
		// This channel is not supporting ACKs since it is unreliable. So do nothing
	}

	bool UnreliableOrderedTransmissionChannel::IsMessageDuplicated( uint16 messageSequenceNumber ) const
	{
		return false;
	}

	void UnreliableOrderedTransmissionChannel::Update( float32 deltaTime, Metrics::MetricsHandler* metrics_handler )
	{
	}

	void UnreliableOrderedTransmissionChannel::Reset()
	{
		TransmissionChannel::Reset();
		_lastMessageSequenceNumberReceived = 0;
	}

	bool UnreliableOrderedTransmissionChannel::IsSequenceNumberNewerThanLastReceived( uint16 sequenceNumber ) const
	{
		// The second part of the if is to support the case when sequence number reaches its limit value and wraps
		// around
		if ( sequenceNumber > _lastMessageSequenceNumberReceived ||
		     ( _lastMessageSequenceNumberReceived - sequenceNumber ) >= HALF_UINT16 )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
} // namespace NetLib
