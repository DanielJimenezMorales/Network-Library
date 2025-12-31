#pragma once
#include "numeric_types.h"

namespace NetLib
{
	class Buffer;

	enum MessageType : uint8
	{
		ConnectionRequest = 0,
		ConnectionAccepted = 1,
		ConnectionDenied = 2,
		ConnectionChallenge = 3,
		ConnectionChallengeResponse = 4,
		Disconnection = 5,
		TimeRequest = 6,
		TimeResponse = 7,
		Replication = 8,
		Inputs = 9,
		PingPong = 10
	};

	struct MessageHeader
	{
			MessageHeader( MessageType messageType, uint16 packetSequenceNumber, bool isReliable, bool isOrdered )
			    : type( messageType )
			    , messageSequenceNumber( packetSequenceNumber )
			    , isReliable( isReliable )
			    , isOrdered( isOrdered )
			{
			}

			MessageHeader( const MessageHeader& other )
			    : type( other.type )
			    , messageSequenceNumber( other.messageSequenceNumber )
			    , isReliable( other.isReliable )
			    , isOrdered( other.isOrdered )
			{
			}

			void Write( Buffer& buffer ) const;
			void Read( Buffer& buffer );
			bool ReadWithoutHeader( Buffer& buffer );
			static uint32 Size() { return sizeof( MessageType ) + sizeof( uint16 ) + sizeof( uint8 ); }

			~MessageHeader() {}

			MessageType type;
			uint16 messageSequenceNumber;
			bool isReliable;
			bool isOrdered;
	};
} // namespace NetLib
