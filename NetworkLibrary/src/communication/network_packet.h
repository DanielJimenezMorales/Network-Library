#pragma once
#include "numeric_types.h"

#include <vector>
#include <memory>

namespace NetLib
{
	class Buffer;
	class Message;

	struct NetworkPacketHeader
	{
			NetworkPacketHeader()
			    : lastAckedSequenceNumber( 0 )
			    , ackBits( 0 )
			    , channelType( 0 )
			{
			}

			NetworkPacketHeader( uint16 ack, uint32 ack_bits, uint8 channel_type )
			    : lastAckedSequenceNumber( ack )
			    , ackBits( ack_bits )
			    , channelType( channel_type )
			{
			}

			void Write( Buffer& buffer ) const;

			void SetACKs( uint32 acks ) { ackBits = acks; };
			void SetHeaderLastAcked( uint16 lastAckedMessage ) { lastAckedSequenceNumber = lastAckedMessage; };
			void SetChannelType( uint8 type ) { channelType = type; };

			uint16 lastAckedSequenceNumber;
			uint32 ackBits;
			uint8 channelType;

			static constexpr uint32 SIZE = sizeof( uint16 ) + sizeof( uint32 ) + sizeof( uint8 );
	};

	class NetworkPacket
	{
		public:
			// NetworkPacket() : _defaultMTUSizeInBytes(1500) {};
			NetworkPacket();
			NetworkPacket( const NetworkPacket& ) = delete;
			NetworkPacket( NetworkPacket&& other ) noexcept = delete;

			~NetworkPacket();

			NetworkPacket& operator=( const NetworkPacket& ) = delete;
			NetworkPacket& operator=( NetworkPacket&& other ) noexcept = delete;

			void Write( Buffer& buffer ) const;

			const NetworkPacketHeader& GetHeader() const { return _header; };

			bool AddMessage( std::unique_ptr< Message > message );
			bool AddMessages( std::vector< std::unique_ptr< Message > >& messages );
			std::unique_ptr< Message > TryGetNextMessage();
			const std::vector< std::unique_ptr< Message > >& GetAllMessages() const;
			uint32 GetNumberOfMessages() const { return static_cast< uint32 >( _messages.size() ); }

			uint32 Size() const;
			uint32 MaxSize() const { return _defaultMTUSizeInBytes; };
			bool CanMessageFit( uint32 sizeOfMessagesInBytes ) const;

			void SetHeader( const NetworkPacketHeader& header ) { _header = header; };
			void SetHeaderACKs( uint32 acks ) { _header.SetACKs( acks ); };
			void SetHeaderLastAcked( uint16 lastAckedMessage ) { _header.SetHeaderLastAcked( lastAckedMessage ); };
			void SetHeaderChannelType( uint8 channelType ) { _header.SetChannelType( channelType ); };

		private:
			const uint32 _defaultMTUSizeInBytes;
			NetworkPacketHeader _header;
			std::vector< std::unique_ptr< Message > > _messages;
	};
} // namespace NetLib
