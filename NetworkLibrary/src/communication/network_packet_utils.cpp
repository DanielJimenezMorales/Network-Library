#include "network_packet_utils.h"

#include "asserts.h"
#include "Logger.h"

#include "core/buffer.h"
#include "communication/network_packet.h"
#include "communication/message_factory.h"
#include "communication/message_utils.h"

#include "transmission_channels/transmission_channel.h"

#include <vector>

namespace NetLib
{
	static bool ReadNetworkPacketHeader( Buffer& buffer, NetworkPacketHeader& out_header )
	{
		if ( buffer.GetRemainingSize() < NetworkPacketHeader::SIZE )
		{
			LOG_ERROR( "Not enough data in buffer to read Network Packet header." );
			return false;
		}

		if ( !buffer.ReadShort( out_header.lastAckedSequenceNumber ) )
		{
			return false;
		}

		if ( !buffer.ReadInteger( out_header.ackBits ) )
		{
			return false;
		}

		if ( !buffer.ReadByte( out_header.channelType ) )
		{
			return false;
		}

		if ( out_header.channelType >= TransmissionChannelType::Count )
		{
			LOG_ERROR( "Invalid channel type %u in Network Packet header.", out_header.channelType );
			return false;
		}

		return true;
	}

	static bool ReadNetworkPacketMessages( Buffer& buffer, MessageFactory& message_factory,
	                                       std::vector< std::unique_ptr< Message > >& out_messages )
	{
		// Read number of messages
		uint8 numberOfMessages;
		if ( !buffer.ReadByte( numberOfMessages ) )
		{
			LOG_ERROR( "Error reading number of messages in Network Packet." );
			return false;
		}

		// Read messages
		uint32 messageCount = 0;
		for ( ; messageCount < numberOfMessages; ++messageCount )
		{
			std::unique_ptr< Message > message = MessageUtils::ReadMessage( message_factory, buffer );
			if ( message != nullptr )
			{
				out_messages.push_back( std::move( message ) );
			}
			else
			{
				LOG_ERROR( "Error reading Network Packet message %u/%u.", messageCount + 1, numberOfMessages );
				break;
			}
		}

		// If not all messages were read succesfully, release the ones that were read
		if ( messageCount < numberOfMessages )
		{
			for ( auto it = out_messages.begin(); it < out_messages.end(); ++it )
			{
				message_factory.ReleaseMessage( std::move( *it ) );
			}
			return false;
		}

		return true;
	}

	bool NetworkPacketUtils::ReadNetworkPacket( Buffer& buffer, MessageFactory& message_factory,
	                                            NetworkPacket& out_packet )
	{
		// Read header
		NetworkPacketHeader header;
		if ( !ReadNetworkPacketHeader( buffer, header ) )
		{
			LOG_ERROR( "Error reading Network Packet header." );
			return false;
		}

		// Read messages
		std::vector< std::unique_ptr< Message > > messages;
		if ( !ReadNetworkPacketMessages( buffer, message_factory, messages ) )
		{
			return false;
		}

		// Configure output packet
		out_packet.SetHeader( header );
		out_packet.AddMessages( std::move( messages ) );
		return true;
	}

	void NetworkPacketUtils::CleanPacket( MessageFactory& message_factory, NetworkPacket& packet )
	{
		const uint32 numberOfMessages = packet.GetNumberOfMessages();
		for ( uint32 i = 0; i < numberOfMessages; ++i )
		{
			std::unique_ptr< Message > message = packet.TryGetNextMessage();
			message_factory.ReleaseMessage( std::move( message ) );
		}

		ASSERT( packet.GetNumberOfMessages() == 0, "Network packet still has messages inside." );
	}
} // namespace NetLib
