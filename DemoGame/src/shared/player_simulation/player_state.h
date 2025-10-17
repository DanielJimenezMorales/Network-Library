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
				movementDirection = Vec2f( 0.f, 0.f );
				isWalking = false;
				isAiming = false;
				timeLeftUntilNextShot = 0.f;
			}

			// Header fields
			uint32 tick;

			// Position
			Vec2f position;

			// Rotation
			float32 rotationAngle;

			// Movement
			Vec2f movementDirection;
			bool isWalking;

			// Aiming
			bool isAiming;

			// Shooting
			float32 timeLeftUntilNextShot;

			bool operator==( const PlayerState& other ) const
			{
				return tick == other.tick && position == other.position && rotationAngle == other.rotationAngle &&
				       isWalking == other.isWalking && other.movementDirection == movementDirection &&
				       isAiming == other.isAiming && timeLeftUntilNextShot == other.timeLeftUntilNextShot;
			}

			bool operator!=( const PlayerState& other ) const { return !operator==( other ); }
	};
} // namespace PlayerSimulation
