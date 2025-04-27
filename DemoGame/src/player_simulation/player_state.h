#pragma once
#include "Vec2f.h"

struct PlayerState
{
		void ZeroOut()
		{
			position = Vec2f( 0.f, 0.f );
			rotationAngle = 0.f;
		}

		Vec2f position;
		float32 rotationAngle;
};
