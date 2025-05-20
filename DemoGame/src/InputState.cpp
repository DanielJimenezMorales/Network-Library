#include "InputState.h"

#include "core/buffer.h"

InputState::InputState()
    : IInputState()
    , tick( 0 )
    , movement( 0.f, 0.f )
    , isShooting( false )
    , virtualMousePosition( 0.f, 0.f )
{
}

int32 InputState::GetSize() const
{
	return ( 1 * sizeof( uint32 ) ) + ( 4 * sizeof( float32 ) ) + ( 1 * sizeof( uint8 ) );
}

void InputState::Serialize( NetLib::Buffer& buffer ) const
{
	buffer.WriteInteger( tick );
	buffer.WriteFloat( movement.X() );
	buffer.WriteFloat( movement.Y() );

	buffer.WriteFloat( virtualMousePosition.X() );
	buffer.WriteFloat( virtualMousePosition.Y() );

	buffer.WriteByte( isShooting ? 1 : 0 );
}

void InputState::Deserialize( NetLib::Buffer& buffer )
{
	tick = buffer.ReadInteger();

	movement.X( buffer.ReadFloat() );
	movement.Y( buffer.ReadFloat() );

	virtualMousePosition.X( buffer.ReadFloat() );
	virtualMousePosition.Y( buffer.ReadFloat() );

	const uint8 isShootingByte = buffer.ReadByte();
	isShooting = ( isShootingByte == 1 ) ? true : false;
}
