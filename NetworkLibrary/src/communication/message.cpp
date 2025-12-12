#include "message.h"

#include "logger.h"

#include "core/buffer.h"

namespace NetLib
{
	void ConnectionRequestMessage::Write( Buffer& buffer ) const
	{
		_header.Write( buffer );
		buffer.WriteLong( clientSalt );
	}

	bool ConnectionRequestMessage::Read( Buffer& buffer )
	{
		_header.type = MessageType::ConnectionRequest;
		if ( !_header.ReadWithoutHeader( buffer ) )
		{
			return false;
		}

		if ( !buffer.ReadLong( clientSalt ) )
		{
			return false;
		}

		return true;
	}

	uint32 ConnectionRequestMessage::Size() const
	{
		return MessageHeader::Size() + sizeof( uint64 );
	}

	void ConnectionChallengeMessage::Write( Buffer& buffer ) const
	{
		_header.Write( buffer );
		buffer.WriteLong( clientSalt );
		buffer.WriteLong( serverSalt );
	}

	bool ConnectionChallengeMessage::Read( Buffer& buffer )
	{
		_header.type = MessageType::ConnectionChallenge;
		if ( !_header.ReadWithoutHeader( buffer ) )
		{
			return false;
		}

		if ( !buffer.ReadLong( clientSalt ) )
		{
			return false;
		}

		if ( !buffer.ReadLong( serverSalt ) )
		{
			return false;
		}

		return true;
	}

	uint32 ConnectionChallengeMessage::Size() const
	{
		return MessageHeader::Size() + ( sizeof( uint64 ) * 2 );
	}

	void ConnectionChallengeResponseMessage::Write( Buffer& buffer ) const
	{
		_header.Write( buffer );
		buffer.WriteLong( prefix );
	}

	bool ConnectionChallengeResponseMessage::Read( Buffer& buffer )
	{
		_header.type = MessageType::ConnectionChallengeResponse;
		if ( !_header.ReadWithoutHeader( buffer ) )
		{
			return false;
		}

		if ( !buffer.ReadLong( prefix ) )
		{
			return false;
		}

		return true;
	}

	uint32 ConnectionChallengeResponseMessage::Size() const
	{
		return MessageHeader::Size() + sizeof( uint64 );
	}

	void ConnectionAcceptedMessage::Write( Buffer& buffer ) const
	{
		_header.Write( buffer );
		buffer.WriteLong( prefix );
		buffer.WriteShort( clientIndexAssigned );
	}

	bool ConnectionAcceptedMessage::Read( Buffer& buffer )
	{
		_header.type = MessageType::ConnectionAccepted;
		if ( !_header.ReadWithoutHeader( buffer ) )
		{
			return false;
		}

		if ( !buffer.ReadLong( prefix ) )
		{
			return false;
		}

		if ( !buffer.ReadShort( clientIndexAssigned ) )
		{
			return false;
		}

		return true;
	}

	uint32 ConnectionAcceptedMessage::Size() const
	{
		return MessageHeader::Size() + sizeof( uint64 ) + sizeof( uint16 );
	}

	void ConnectionDeniedMessage::Write( Buffer& buffer ) const
	{
		_header.Write( buffer );
		buffer.WriteByte( reason );
	}

	bool ConnectionDeniedMessage::Read( Buffer& buffer )
	{
		_header.type = MessageType::ConnectionDenied;
		if ( !_header.ReadWithoutHeader( buffer ) )
		{
			return false;
		}

		if ( !buffer.ReadByte( reason ) )
		{
			return false;
		}

		return true;
	}

	uint32 ConnectionDeniedMessage::Size() const
	{
		return MessageHeader::Size() + sizeof( uint8 );
	}

	void DisconnectionMessage::Write( Buffer& buffer ) const
	{
		_header.Write( buffer );
		buffer.WriteLong( prefix );
		buffer.WriteByte( reason );
	}

	bool DisconnectionMessage::Read( Buffer& buffer )
	{
		_header.type = MessageType::Disconnection;
		if ( !_header.ReadWithoutHeader( buffer ) )
		{
			return false;
		}

		if ( !buffer.ReadLong( prefix ) )
		{
			return false;
		}

		if ( !buffer.ReadByte( reason ) )
		{
			return false;
		}

		return true;
	}

	uint32 DisconnectionMessage::Size() const
	{
		return MessageHeader::Size() + sizeof( uint64 ) + sizeof( uint8 );
	}

	void TimeRequestMessage::Write( Buffer& buffer ) const
	{
		_header.Write( buffer );
		buffer.WriteInteger( remoteTime );
	}

	bool TimeRequestMessage::Read( Buffer& buffer )
	{
		_header.type = MessageType::TimeRequest;
		if ( !_header.ReadWithoutHeader( buffer ) )
		{
			return false;
		}

		if ( !buffer.ReadInteger( remoteTime ) )
		{
			return false;
		}

		return true;
	}

	uint32 TimeRequestMessage::Size() const
	{
		return MessageHeader::Size() + sizeof( uint32 );
	}

	void TimeResponseMessage::Write( Buffer& buffer ) const
	{
		_header.Write( buffer );
		buffer.WriteInteger( remoteTime );
		buffer.WriteInteger( serverTime );
	}

	bool TimeResponseMessage::Read( Buffer& buffer )
	{
		_header.type = MessageType::TimeResponse;
		if ( !_header.ReadWithoutHeader( buffer ) )
		{
			return false;
		}

		if ( !buffer.ReadInteger( remoteTime ) )
		{
			return false;
		}

		if ( !buffer.ReadInteger( serverTime ) )
		{
			return false;
		}

		return true;
	}

	uint32 TimeResponseMessage::Size() const
	{
		return MessageHeader::Size() + ( 2 * sizeof( uint32 ) );
	}

	void ReplicationMessage::Write( Buffer& buffer ) const
	{
		_header.Write( buffer );
		buffer.WriteByte( replicationAction );
		buffer.WriteInteger( networkEntityId );
		buffer.WriteInteger( controlledByPeerId );
		buffer.WriteInteger( replicatedClassId );
		buffer.WriteShort( dataSize );
		if ( dataSize > 0 )
		{
			buffer.WriteData( data, dataSize );
		}
	}

	bool ReplicationMessage::Read( Buffer& buffer )
	{
		_header.type = MessageType::Replication;
		if ( !_header.ReadWithoutHeader( buffer ) )
		{
			return false;
		}

		if ( !buffer.ReadByte( replicationAction ) )
		{
			return false;
		}

		if ( !buffer.ReadInteger( networkEntityId ) )
		{
			return false;
		}

		if ( !buffer.ReadInteger( controlledByPeerId ) )
		{
			return false;
		}

		if ( !buffer.ReadInteger( replicatedClassId ) )
		{
			return false;
		}

		if ( !buffer.ReadShort( dataSize ) )
		{
			return false;
		}

		if ( dataSize > 0 )
		{
			data = new uint8[ dataSize ];
			if ( !buffer.ReadData( data, dataSize ) )
			{
				return false;
			}
		}

		return true;
	}

	uint32 ReplicationMessage::Size() const
	{
		return MessageHeader::Size() + sizeof( uint8 ) + ( 3 * sizeof( uint32 ) ) + sizeof( uint16 ) +
		       ( dataSize * sizeof( uint8 ) );
	}

	void ReplicationMessage::Reset()
	{
		if ( data != nullptr )
		{
			delete[] data;
			data = nullptr;
		}
	}

	ReplicationMessage::~ReplicationMessage()
	{
		if ( data != nullptr )
		{
			delete[] data;
			data = nullptr;
		}
	}

	void InputStateMessage::Write( Buffer& buffer ) const
	{
		_header.Write( buffer );

		buffer.WriteShort( dataSize );
		buffer.WriteData( data, dataSize );
	}

	bool InputStateMessage::Read( Buffer& buffer )
	{
		_header.type = MessageType::Inputs;
		if ( !_header.ReadWithoutHeader( buffer ) )
		{
			return false;
		}

		if ( !buffer.ReadShort( dataSize ) )
		{
			return false;
		}

		if ( dataSize > 0 )
		{
			data = new uint8[ dataSize ];
		}

		if ( !buffer.ReadData( data, dataSize ) )
		{
			return false;
		}

		return true;
	}

	uint32 InputStateMessage::Size() const
	{
		return MessageHeader::Size() + sizeof( uint16 ) + ( dataSize * sizeof( uint8 ) );
	}

	void InputStateMessage::Reset()
	{
		if ( data != nullptr )
		{
			delete[] data;
			data = nullptr;
		}
	}

	void PingPongMessage::Write( Buffer& buffer ) const
	{
		_header.Write( buffer );
	}

	bool PingPongMessage::Read( Buffer& buffer )
	{
		_header.type = MessageType::PingPong;
		if ( !_header.ReadWithoutHeader( buffer ) )
		{
			return false;
		}

		return true;
	}

	uint32 PingPongMessage::Size() const
	{
		return MessageHeader::Size();
	}

	void PingPongMessage::Reset()
	{
	}
} // namespace NetLib
