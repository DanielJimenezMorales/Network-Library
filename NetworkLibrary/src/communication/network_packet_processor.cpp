#include "network_packet_processor.h"

#include "logger.h"
#include "asserts.h"

#include "core/buffer.h"
#include "communication/network_packet.h"
#include "communication/message_factory.h"
#include "communication/message_utils.h"

namespace NetLib
{
	NetworkPacketProcessor::NetworkPacketProcessor()
	    : _messageFactory( nullptr )
	    , _isInitialized( false )
	{
	}

	void NetworkPacketProcessor::Initialize( MessageFactory* message_factory )
	{
		ASSERT( message_factory != nullptr, "Message factory pointer can't be null." );

		_messageFactory = message_factory;
		_isInitialized = true;
	}

	static bool ReadPacketHeaderFromBuffer( Buffer& buffer, NetworkPacketHeader& out_header )
	{
		// TODO Check if size left for being read is at least equal to header size
		out_header.lastAckedSequenceNumber = buffer.ReadShort();
		out_header.ackBits = buffer.ReadInteger();
		out_header.channelType = buffer.ReadByte();

		return true;
	}

	bool NetworkPacketProcessor::ReadPacket( Buffer& buffer, NetworkPacket& out_packet ) const
	{
		ASSERT( _isInitialized, "NetworkPacketProcessor is not initialized." );

		// Read packet header
		NetworkPacketHeader packetHeader;
		bool result = ReadPacketHeaderFromBuffer( buffer, packetHeader );
		if ( !result )
		{
			LOG_ERROR( "Error reading network packet header from buffer." );
			return false;
		}
		out_packet.SetHeader( packetHeader );

		// Read packet messages
		bool readAllMessages = true;
		const uint8 numberOfMessages = buffer.ReadByte();
		for ( uint32 i = 0; i < numberOfMessages; ++i )
		{
			std::unique_ptr< Message > message = MessageUtils::ReadMessage( *_messageFactory, buffer );
			if ( message == nullptr )
			{
				readAllMessages = false;
				break;
			}
			else
			{
				out_packet.AddMessage( std::move( message ) );
			}
		}

		if ( !readAllMessages )
		{
			LOG_ERROR( "Error reading one or more messages from network packet buffer." );
			return false;
		}

		return true;
	}

	bool NetworkPacketProcessor::WritePacket( const NetworkPacket& packet, Buffer& out_buffer ) const
	{
		return false;
	}

	void NetworkPacketProcessor::CleanPacket( NetworkPacket& packet ) const
	{
		ASSERT( _isInitialized, "NetworkPacketProcessor is not initialized." );

		const uint32 numberOfMessages = packet.GetNumberOfMessages();
		for ( uint32 i = 0; i < numberOfMessages; ++i )
		{
			std::unique_ptr< Message > message = packet.TryGetNextMessage();
			_messageFactory->ReleaseMessage( std::move( message ) );
		}

		ASSERT( packet.GetNumberOfMessages() == 0, "Network packet still has messages inside." );
	}
} // namespace NetLib
