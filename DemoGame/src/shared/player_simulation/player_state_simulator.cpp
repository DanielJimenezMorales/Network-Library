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
	    , _lastSimulationEvents()
	{
	}

	PlayerState PlayerStateSimulator::Simulate( const InputState& inputs, const PlayerState& current_state,
	                                            const PlayerStateConfiguration& configuration, float32 elapsed_time )
	{
		_lastSimulationEvents.clear();

		PlayerState playerState;
		playerState.tick = inputs.tick;

		_movementController.Simulate( inputs, current_state, playerState, elapsed_time, configuration,
		                              _lastSimulationEvents );
		_rotationController.Simulate( inputs, current_state, playerState, elapsed_time, configuration,
		                              _lastSimulationEvents );
		_shootingController.Simulate( inputs, current_state, playerState, elapsed_time, configuration,
		                              _lastSimulationEvents );

		return playerState;
	}

	void PlayerStateSimulator::ProcessLastSimulationEvents( Engine::ECS::World& world, Engine::ECS::GameEntity& entity,
	                                                        IPlayerSimulationEventsProcessor* events_processor )
	{
		assert( entity.IsValid() );
		assert( events_processor != nullptr );

		for ( auto cit = _lastSimulationEvents.cbegin(); cit != _lastSimulationEvents.cend(); ++cit )
		{
			const bool result = events_processor->ProcessEvent( world, entity, *cit );
			if ( !result )
			{
				LOG_ERROR( "Failed to process simulation event of type %u", *cit );
			}
		}
	}
} // namespace PlayerSimulation