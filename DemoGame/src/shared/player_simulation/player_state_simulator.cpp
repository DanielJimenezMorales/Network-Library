#include "player_state_simulator.h"

#include <cassert>

#include "logger.h"

#include "ecs/world.h"
#include "ecs/game_entity.hpp"

#include "shared/InputState.h"

#include "shared/player_simulation/player_state.h"
#include "shared/player_simulation/i_player_simulation_events_processor.h"

namespace PlayerSimulation
{
	PlayerStateSimulator::PlayerStateSimulator()
	    : _movementController()
	    , _rotationController()
	    , _shootingController()
	    , _simulationEventsHandler()
	{
	}

	PlayerState PlayerStateSimulator::Simulate( const InputState& inputs, const PlayerState& current_state,
	                                            const PlayerStateConfiguration& configuration, float32 elapsed_time )
	{
		_simulationEventsHandler.Clear();

		PlayerState playerState;
		playerState.tick = inputs.tick;

		_movementController.Simulate( inputs, current_state, playerState, elapsed_time, configuration,
		                              _simulationEventsHandler );
		_rotationController.Simulate( inputs, current_state, playerState, elapsed_time, configuration,
		                              _simulationEventsHandler );
		_shootingController.Simulate( inputs, current_state, playerState, elapsed_time, configuration,
		                              _simulationEventsHandler );

		return playerState;
	}

	void PlayerStateSimulator::ProcessLastSimulationEvents( Engine::ECS::World& world, Engine::ECS::GameEntity& entity,
	                                                        IPlayerSimulationEventsProcessor* events_processor )
	{
		_simulationEventsHandler.ProcessEvents( world, entity, events_processor );
	}
} // namespace PlayerSimulation