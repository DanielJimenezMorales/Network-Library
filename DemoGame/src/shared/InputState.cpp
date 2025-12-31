#include "InputState.h"

#include "logger.h"

#include "core/buffer.h"

InputState::InputState()
    : IInputState()
    , tick( 0 )
    , serverTime( 0.f )
    , movement( 0.f, 0.f )
    , isShooting( false )
    , virtualMousePosition( 0.f, 0.f )
{
}

int32 InputState::GetSize() const
{
	return ( 1 * sizeof( uint32 ) ) + ( 5 * sizeof( float32 ) ) + ( 2 * sizeof( uint8 ) );
}

void InputState::Serialize( NetLib::Buffer& buffer ) const
{
	buffer.WriteInteger( tick );
	buffer.WriteFloat( serverTime );

	buffer.WriteFloat( movement.X() );
	buffer.WriteFloat( movement.Y() );

	buffer.WriteFloat( virtualMousePosition.X() );
	buffer.WriteFloat( virtualMousePosition.Y() );

	buffer.WriteByte( isAiming ? 1 : 0 );
	buffer.WriteByte( isShooting ? 1 : 0 );
}

bool InputState::Deserialize( NetLib::Buffer& buffer )
{
	if ( buffer.GetRemainingSize() < GetSize() )
	{
		LOG_ERROR( "Not enough data in buffer to read InputState." );
		return false;
	}

	tick = buffer.ReadInteger();
	serverTime = buffer.ReadFloat();

	movement.X( buffer.ReadFloat() );
	movement.Y( buffer.ReadFloat() );

	if ( movement.X() >= MAXIMUM_MOVEMENT_VALUE || movement.Y() >= MAXIMUM_MOVEMENT_VALUE )
	{
		LOG_ERROR( "[InputState::%s] Movement value too high, possible cheating detected. X: %.3f, Y: %.3f",
		           THIS_FUNCTION_NAME, movement.X(), movement.Y() );
		return false;
	}

	virtualMousePosition.X( buffer.ReadFloat() );
	virtualMousePosition.Y( buffer.ReadFloat() );

	const uint8 isAimingByte = buffer.ReadByte();
	isAiming = ( isAimingByte == 1 ) ? true : false;

	const uint8 isShootingByte = buffer.ReadByte();
	isShooting = ( isShootingByte == 1 ) ? true : false;

	return true;
}
