#pragma once
#include "numeric_types.h"

namespace Engine::ECS
{
	class GameEntity;
}

struct PlayerInterpolatedState;

class PlayerInterpolationPositionAndRotationApplier
{
	public:
		void Execute( Engine::ECS::GameEntity& position_entity, Engine::ECS::GameEntity& rotation_entity,
		              const PlayerInterpolatedState& state, float32 elapsed_time );
};
