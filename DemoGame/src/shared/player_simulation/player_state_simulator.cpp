#include "player_state_simulator.h"

#include <cassert>

#include "logger.h"

#include "ecs/world.h"
#include "ecs/game_entity.hpp"

#include "shared/InputState.h"

#include "shared/player_simulation/player_state.h"
#include "shared/player_simulation/i_simulation_events_processor.h"

namespace PlayerSimulation
{
	PlayerState PlayerStateSimulator::Simulate( const InputState& inputs, const PlayerState& current_state,
	                                            const PlayerStateConfiguration& configuration, float32 elapsed_time )
	{
		LOG_INFO( "[%s] Start player simulation", THIS_FUNCTION_NAME );
		_simulationEventsHandler.Clear();

		PlayerState playerState;
		playerState.tick = inputs.tick;

		_movementController.Simulate( inputs, current_state, playerState, elapsed_time, configuration,
		                              _simulationEventsHandler );
		_rotationController.Simulate( inputs, current_state, playerState, elapsed_time, configuration,
		                              _simulationEventsHandler );
		_shootingController.Simulate( inputs, current_state, playerState, elapsed_time, configuration,
		                              _simulationEventsHandler );

		LOG_INFO( "[%s] End player simulation", THIS_FUNCTION_NAME );
		return playerState;
	}

	void PlayerStateSimulator::ProcessLastSimulationEvents( Engine::ECS::World& world, Engine::ECS::GameEntity& entity,
	                                                        ISimulationEventsProcessor* events_processor )
	{
		LOG_INFO( "[%s] Process events from last simulation", THIS_FUNCTION_NAME );
		_simulationEventsHandler.ProcessEvents( world, entity, events_processor );
	}
} // namespace PlayerSimulation