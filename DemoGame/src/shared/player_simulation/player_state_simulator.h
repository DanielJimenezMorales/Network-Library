#pragma once
#include "numeric_types.h"

#include "shared/player_simulation/player_simulation_events.h"
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
struct PlayerState;
struct PlayerStateConfiguration;

namespace PlayerSimulation
{
	class IPlayerSimulationEventsProcessor;

	class PlayerStateSimulator
	{
		public:
			PlayerStateSimulator();

			PlayerState Simulate( const InputState& inputs, const PlayerState& current_state,
			                      const PlayerStateConfiguration& configuration, float32 elapsed_time );

			void ProcessLastSimulationEvents( Engine::ECS::World& world, Engine::ECS::GameEntity& entity,
			                                  IPlayerSimulationEventsProcessor* events_processor );

		private:
			PlayerMovementController _movementController;
			PlayerRotationController _rotationController;
			PlayerShootingController _shootingController;

			// TODO Encapsulate this under a class
			std::vector< EventType > _lastSimulationEvents;
	};
}