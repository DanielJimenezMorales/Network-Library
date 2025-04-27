#pragma once
#include "numeric_types.h"

#include "player_simulation/player_movement_controller.h"
#include "player_simulation/player_rotation_controller.h"
#include "player_simulation/player_state_configuration.h"

namespace ECS
{
	class World;
}

class InputState;
struct PlayerState;

class PlayerStateSimulator
{
	public:
		PlayerStateSimulator( ECS::World* world, const PlayerStateConfiguration& configuration );

		bool Simulate( const InputState& inputs, const PlayerState& current_state, PlayerState& result_state,
		               float32 elapsed_time ) const;

	private:
		ECS::World* _world;

		const PlayerStateConfiguration _configuration;

		PlayerMovementController _movementController;
		PlayerRotationController _rotationController;
};
