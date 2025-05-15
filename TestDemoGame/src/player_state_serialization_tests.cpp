#include "gtest/gtest.h"

#include "core/Buffer.h"

#include "player_simulation/player_state.h"
#include "player_simulation/player_state_utils.h"

namespace
{
	TEST( PlayerStateSerialization, SerializeAndDeserializeEqualPlayerState )
	{
		PlayerState playerState;
		playerState.tick = 11;
		playerState.position = Vec2f( 2.45f, -1.4f );
		playerState.rotationAngle = 99.f;

		uint8* bufferData = new uint8[ 1024 ];
		NetLib::Buffer buffer( bufferData, 1024 );

		SerializePlayerStateToBuffer( playerState, buffer );
		buffer.ResetAccessIndex();
		PlayerState resultPlayerState = DeserializePlayerStateFromBuffer( buffer );

		EXPECT_EQ( playerState.tick, resultPlayerState.tick );
		EXPECT_FLOAT_EQ( playerState.position.X(), resultPlayerState.position.X() );
		EXPECT_FLOAT_EQ( playerState.position.Y(), resultPlayerState.position.Y() );
		EXPECT_FLOAT_EQ( playerState.rotationAngle, resultPlayerState.rotationAngle );

		delete[] bufferData;
	}
}