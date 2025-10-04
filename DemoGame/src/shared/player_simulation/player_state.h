#pragma once
#include "vec2f.h"

namespace PlayerSimulation
{
	struct PlayerState
	{
			void ZeroOut()
			{
				tick = 0;
				position = Vec2f( 0.f, 0.f );
				rotationAngle = 0.f;
				timeLeftUntilNextShot = 0.f;
			}

			// Header fields
			uint32 tick;

			// Position
			Vec2f position;

			// Rotation
			float32 rotationAngle;

			// Movement
			bool isWalking;

			// Shooting
			float32 timeLeftUntilNextShot;

			bool operator==( const PlayerState& other ) const
			{
				return tick == other.tick && position == other.position && rotationAngle == other.rotationAngle &&
				       isWalking == other.isWalking && timeLeftUntilNextShot == other.timeLeftUntilNextShot;
			}

			bool operator!=( const PlayerState& other ) const { return !operator==( other ); }
	};
} // namespace PlayerSimulation
