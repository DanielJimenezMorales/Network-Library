#pragma once
#include "numeric_types.h"

namespace Engine::ECS
{
	class GameEntity;
}

struct PlayerInterpolatedState;

class PlayerWeaponFlipApplier
{
	public:
		void Execute( Engine::ECS::GameEntity& entity, const PlayerInterpolatedState& state, float32 elapsed_time );
};
