#include "player_state_simulator.h"

#include "ecs/world.h"

#include "InputState.h"

#include "player_simulation/player_state.h"

PlayerStateSimulator::PlayerStateSimulator( ECS::World* world )
    : _world( world )
    , _movementController()
    , _rotationController()
{
}

bool PlayerStateSimulator::Simulate( const InputState& inputs, const PlayerState& current_state,
                                     PlayerState& result_state, const PlayerStateConfiguration& configuration,
                                     float32 elapsed_time ) const
{
	result_state.ZeroOut();

	_movementController.Simulate( inputs, current_state, result_state, elapsed_time, configuration);
	_rotationController.Simulate( inputs, current_state, result_state, elapsed_time, configuration);

	return true;
}
