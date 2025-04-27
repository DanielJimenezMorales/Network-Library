#pragma once
#include "numeric_types.h"

#include "player_simulation/player_movement_controller.h"
#include "player_simulation/player_rotation_controller.h"
#include "player_simulation/player_shooting_controller.h"

namespace ECS
{
	class World;
	class GameEntity;
}

class InputState;
struct PlayerState;
struct PlayerStateConfiguration;

class PlayerStateSimulator
{
	public:
		PlayerStateSimulator();

		void Configure( const ECS::World* world, const ECS::GameEntity& player_entity );

		bool Simulate( const InputState& inputs, const PlayerState& current_state, PlayerState& result_state,
		               const PlayerStateConfiguration& configuration, float32 elapsed_time );

	private:
		PlayerMovementController _movementController;
		PlayerRotationController _rotationController;
		PlayerShootingController _shootingController;
};
