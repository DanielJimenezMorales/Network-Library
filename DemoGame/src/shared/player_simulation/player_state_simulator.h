#pragma once
#include "numeric_types.h"

#include "shared/player_simulation/simulation_events_handler.h"
#include "shared/player_simulation/player_movement_controller.h"
#include "shared/player_simulation/player_rotation_controller.h"
#include "shared/player_simulation/player_shooting_controller.h"

#include <vector>

namespace Engine
{
	namespace ECS
	{
		class World;
		class GameEntity;
	}
}

class InputState;

namespace PlayerSimulation
{
	class ISimulationEventsProcessor;
	struct PlayerState;
	struct PlayerStateConfiguration;

	class PlayerStateSimulator
	{
		public:
			PlayerStateSimulator() = default;

			PlayerState Simulate( const InputState& inputs, const PlayerState& current_state,
			                      const PlayerStateConfiguration& configuration, float32 elapsed_time );

			void ProcessLastSimulationEvents( Engine::ECS::World& world, Engine::ECS::GameEntity& entity,
			                                  ISimulationEventsProcessor* events_processor );

		private:
			PlayerMovementController _movementController;
			PlayerRotationController _rotationController;
			PlayerShootingController _shootingController;

			SimulationEventsHandler _simulationEventsHandler;
	};
}