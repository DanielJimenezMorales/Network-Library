#include "player_state_simulator.h"

#include "ecs/world.h"

#include "InputState.h"

#include "player_simulation/player_state.h"

PlayerStateSimulator::PlayerStateSimulator()
    : _movementController()
    , _rotationController()
    , _shootingController()
{
}

void PlayerStateSimulator::Configure( const ECS::World* world, const ECS::GameEntity& player_entity )
{
}

bool PlayerStateSimulator::Simulate( const InputState& inputs, const PlayerState& current_state,
                                     PlayerState& result_state, const PlayerStateConfiguration& configuration,
                                     float32 elapsed_time )
{
	result_state.ZeroOut();

	_movementController.Simulate( inputs, current_state, result_state, elapsed_time, configuration );
	_rotationController.Simulate( inputs, current_state, result_state, elapsed_time, configuration );
	_shootingController.Simulate( inputs, current_state, result_state, elapsed_time, configuration );

	return true;
}

void PlayerStateSimulator::UnsubscribeFromOnShotPerformed( uint32 id )
{
	_shootingController.OnShotPerformed.DeleteSubscriber( id );
}
