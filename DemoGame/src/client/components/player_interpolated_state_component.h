#pragma once

#include "numeric_types.h"
#include "Vec2f.h"

enum class PlayerInterpolationAlgorithm
{
	LOCAL_PLAYER = 0,
	REMOTE_PLAYER = 1
};

struct PlayerInterpolatedState
{
		Vec2f position;
		Vec2f movementDirection;
		float32 rotationAngle;
		bool isWalking;
		bool isAiming;
};

struct PlayerInterpolatedStateComponent
{
	public:
		PlayerInterpolatedState state;
		PlayerInterpolationAlgorithm interpolationAlgorithm;
};
