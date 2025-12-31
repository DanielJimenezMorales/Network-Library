#include "network_packet.h"

#include "asserts.h"
#include "Logger.h"

#include "core/buffer.h"

#include "communication/message.h"
#include "communication/message_utils.h"

namespace NetLib
{
	NetworkPacket::~NetworkPacket()
	{
		ASSERT(
		    _messages.empty(),
		    "Before destroying a Network Packet make sure you handle correctly it's packets memory to avoid leaks." );
	}

	void NetworkPacketHeader::Write( Buffer& buffer ) const
	{
		buffer.WriteShort( lastAckedSequenceNumber );
		buffer.WriteInteger( ackBits );
		buffer.WriteByte( channelType );
	}

	NetworkPacket::NetworkPacket()
	    : _header( 0, 0, 0 )
	    , _defaultMTUSizeInBytes( 1500 )
	{
	}

	void NetworkPacket::Write( Buffer& buffer ) const
	{
		_header.Write( buffer );

		const uint8 numberOfMessages = static_cast< uint8 >( _messages.size() );
		buffer.WriteByte( numberOfMessages );

		for ( auto cit = _messages.cbegin(); cit != _messages.cend(); ++cit )
		{
			const Message* message = ( *cit ).get();
			message->Write( buffer );
		}
	}

	bool NetworkPacket::AddMessage( std::unique_ptr< Message > message )
	{
		_messages.push_back( std::move( message ) );
		return true;
	}

	bool NetworkPacket::AddMessages( std::vector< std::unique_ptr< Message > >& messages )
	{
		for ( auto it = messages.begin(); it != messages.end(); ++it )
		{
			AddMessage( std::move( *it ) );
		}

		messages.clear();
		return true;
	}

	std::unique_ptr< Message > NetworkPacket::TryGetNextMessage()
	{
		if ( GetNumberOfMessages() == 0 )
		{
			return nullptr;
		}

		std::unique_ptr< Message > message = std::move( _messages.front() );
		_messages.erase( _messages.begin() );
		return std::move( message );
	}

	const std::vector< std::unique_ptr< Message > >& NetworkPacket::GetAllMessages() const
	{
		return _messages;
	}

	uint32 NetworkPacket::Size() const
	{
		uint32 packetSize = NetworkPacketHeader::SIZE;
		packetSize += 1; // We store in 1 byte the number of messages that this packet contains

		auto iterator = _messages.cbegin();
		while ( iterator != _messages.cend() )
		{
			packetSize += ( *iterator )->Size();
			++iterator;
		}

		return packetSize;
	}

	bool NetworkPacket::CanMessageFit( uint32 sizeOfMessagesInBytes ) const
	{
		return ( sizeOfMessagesInBytes + Size() < MaxSize() );
	}
} // namespace NetLib
