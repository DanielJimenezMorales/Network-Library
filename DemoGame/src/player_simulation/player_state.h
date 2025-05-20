#pragma once
#include "Vec2f.h"

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

		// Body fields
		Vec2f position;
		float32 rotationAngle;

		float32 timeLeftUntilNextShot;

		bool operator==( const PlayerState& other ) const
		{
			return tick == other.tick && position == other.position && rotationAngle == other.rotationAngle;
		}

		bool operator!=( const PlayerState& other ) const { return !operator==( other ); }
};
