#include "unreliable_unordered_transmission_channel.h"

#include "communication/message_factory.h"
#include "communication/network_packet.h"

#include "core/buffer.h"
#include "core/socket.h"
#include "core/address.h"

#include "metrics/metric_names.h"
#include "metrics/metrics_handler.h"

namespace NetLib
{
	UnreliableUnorderedTransmissionChannel::UnreliableUnorderedTransmissionChannel()
	    : TransmissionChannel( TransmissionChannelType::UnreliableUnordered )
	{
	}

	UnreliableUnorderedTransmissionChannel::UnreliableUnorderedTransmissionChannel(
	    UnreliableUnorderedTransmissionChannel&& other ) noexcept
	    : TransmissionChannel( std::move( other ) )
	{
	}

	UnreliableUnorderedTransmissionChannel& UnreliableUnorderedTransmissionChannel::operator=(
	    UnreliableUnorderedTransmissionChannel&& other ) noexcept
	{
		TransmissionChannel::operator=( std::move( other ) );
		return *this;
	}

	bool UnreliableUnorderedTransmissionChannel::GenerateAndSerializePacket( Socket& socket, const Address& address,
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
		bool arePendingMessages = ArePendingMessagesToSend();
		bool isThereCapacityLeft = packet.CanMessageFit( GetSizeOfNextUnsentMessage() );

		while ( arePendingMessages && isThereCapacityLeft )
		{
			// Configure and add message to packet
			std::unique_ptr< Message > message = GetMessageToSend( metrics_handler );

			packet.AddMessage( std::move( message ) );

			// Check if we should include another message to the packet
			arePendingMessages = ArePendingMessagesToSend();
			isThereCapacityLeft = packet.CanMessageFit( GetSizeOfNextUnsentMessage() );
		}

		// Set packet header
		packet.SetHeaderACKs( 0 );
		packet.SetHeaderLastAcked( 0 );
		packet.SetHeaderChannelType( GetType() );

		uint8* bufferData = new uint8[ packet.Size() ];
		Buffer buffer( bufferData, packet.Size() );
		packet.Write( buffer );
		socket.SendTo( buffer.GetData(), buffer.GetSize(), address );

		// TODO See what happens when the socket couldn't send the packet
		if (metrics_handler != nullptr)
		{
			metrics_handler->AddValue(Metrics::UPLOAD_BANDWIDTH_METRIC, packet.Size());
		}

		// Send messages ownership back to remote peer
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

	void UnreliableUnorderedTransmissionChannel::AddMessageToSend( std::unique_ptr< Message > message )
	{
		_unsentMessages.push_back( std::move( message ) );
	}

	bool UnreliableUnorderedTransmissionChannel::ArePendingMessagesToSend() const
	{
		return ( !_unsentMessages.empty() );
	}

	std::unique_ptr< Message > UnreliableUnorderedTransmissionChannel::GetMessageToSend(
	    Metrics::MetricsHandler* metrics_handler )
	{
		if ( !ArePendingMessagesToSend() )
		{
			return nullptr;
		}

		// TODO Check this. This is not a linked list so if you always get and delete the first element you could not
		// have access to the rest in cse there are more
		std::unique_ptr< Message > message( std::move( _unsentMessages[ 0 ] ) );
		_unsentMessages.erase( _unsentMessages.begin() );

		message->SetHeaderPacketSequenceNumber( 0 );

		return std::move( message );
	}

	uint32 UnreliableUnorderedTransmissionChannel::GetSizeOfNextUnsentMessage() const
	{
		if ( !ArePendingMessagesToSend() )
		{
			return 0;
		}

		return _unsentMessages.front()->Size();
	}

	void UnreliableUnorderedTransmissionChannel::AddReceivedMessage( std::unique_ptr< Message > message,
	                                                                 Metrics::MetricsHandler* metrics_handler )
	{
		_readyToProcessMessages.push( std::move( message ) );
	}

	bool UnreliableUnorderedTransmissionChannel::ArePendingReadyToProcessMessages() const
	{
		return ( !_readyToProcessMessages.empty() );
	}

	const Message* UnreliableUnorderedTransmissionChannel::GetReadyToProcessMessage()
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

	void UnreliableUnorderedTransmissionChannel::ProcessACKs( uint32 acks, uint16 lastAckedMessageSequenceNumber,
	                                                          Metrics::MetricsHandler* metrics_handler )
	{
	}

	bool UnreliableUnorderedTransmissionChannel::IsMessageDuplicated( uint16 messageSequenceNumber ) const
	{
		return false;
	}

	void UnreliableUnorderedTransmissionChannel::Update( float32 deltaTime )
	{
	}
} // namespace NetLib
