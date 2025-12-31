#include "message_header.h"

#include "core/buffer.h"

#include "utils/bitwise_utils.h"

namespace NetLib
{
	void MessageHeader::Write( Buffer& buffer ) const
	{
		buffer.WriteByte( type );
		buffer.WriteShort( messageSequenceNumber );

		uint8 flags = 0;
		if ( isReliable )
		{
			BitwiseUtils::SetBitAtIndex( flags, 0 );
		}

		if ( isOrdered )
		{
			BitwiseUtils::SetBitAtIndex( flags, 1 );
		}

		buffer.WriteByte( flags );
	}

	void MessageHeader::Read( Buffer& buffer )
	{
		type = static_cast< MessageType >( buffer.ReadByte() );
		ReadWithoutHeader( buffer );
	}

	bool MessageHeader::ReadWithoutHeader( Buffer& buffer )
	{
		if ( !buffer.ReadShort( messageSequenceNumber ) )
		{
			return false;
		}

		uint8 flags;
		if ( !buffer.ReadByte( flags ) )
		{
			return false;
		}

		isReliable = BitwiseUtils::GetBitAtIndex( flags, 0 );
		isOrdered = BitwiseUtils::GetBitAtIndex( flags, 1 );

		return true;
	}
} // namespace NetLib
