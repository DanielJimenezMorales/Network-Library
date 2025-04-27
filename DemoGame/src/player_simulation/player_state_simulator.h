#pragma once
#include "numeric_types.h"

#include "player_simulation/player_movement_controller.h"
#include "player_simulation/player_rotation_controller.h"

namespace ECS
{
	class World;
}

class InputState;
struct PlayerState;
struct PlayerStateConfiguration;

class PlayerStateSimulator
{
	public:
		PlayerStateSimulator( ECS::World* world );

		bool Simulate( const InputState& inputs, const PlayerState& current_state, PlayerState& result_state,
		               const PlayerStateConfiguration& configuration, float32 elapsed_time ) const;

	private:
		ECS::World* _world;

		PlayerMovementController _movementController;
		PlayerRotationController _rotationController;
};
