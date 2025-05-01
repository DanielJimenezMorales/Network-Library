#include "InputState.h"

#include "core/buffer.h"

int32 InputState::GetSize() const
{
	return ( 1 * sizeof( uint64 ) ) + ( 4 * sizeof( float32 ) ) + ( 1 * sizeof( uint8 ) );
}

void InputState::Serialize( NetLib::Buffer& buffer ) const
{
	buffer.WriteLong( id );
	buffer.WriteFloat( movement.X() );
	buffer.WriteFloat( movement.Y() );

	buffer.WriteFloat( virtualMousePosition.X() );
	buffer.WriteFloat( virtualMousePosition.Y() );

	buffer.WriteByte( isShooting ? 1 : 0 );
}

void InputState::Deserialize( NetLib::Buffer& buffer )
{
	id = buffer.ReadLong();

	movement.X( buffer.ReadFloat() );
	movement.Y( buffer.ReadFloat() );

	virtualMousePosition.X( buffer.ReadFloat() );
	virtualMousePosition.Y( buffer.ReadFloat() );

	const uint8 isShootingByte = buffer.ReadByte();
	isShooting = ( isShootingByte == 1 ) ? true : false;
}
